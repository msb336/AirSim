#include "WorldSimApi.h"
#include "common/common_utils/Utils.hpp"
#include "Weather/WeatherLib.h"


WorldSimApi::WorldSimApi(ASimModeBase* simmode)
    : simmode_(simmode)
{
}

bool WorldSimApi::loadLevel(const std::string& level_name)
{
	bool success;
	using namespace std::chrono_literals;
	// Add loading screen to viewport
	simmode_->toggleLoadingScreen(true);
	pause(true);

	UAirBlueprintLib::RunCommandOnGameThread([this, level_name]() {
		this->current_level_ = UAirBlueprintLib::loadLevel(this->simmode_->GetWorld(), level_name);
	}, true);

	if (this->current_level_)
	{
		success = true;
		std::this_thread::sleep_for(2s);
		spawnPlayer();
	}
	else
		success = false;

	std::this_thread::sleep_for(0.5s);
	//Remove Loading screen from viewport
	simmode_->toggleLoadingScreen(false);
	pause(false);

	return success;
}

void WorldSimApi::spawnPlayer()
{
	using namespace std::chrono_literals;
	UE_LOG(LogTemp, Log, TEXT("spawning player"));
	bool success{ false };
	int counter{ 10 };
	while (!success && counter--)
	{
		UAirBlueprintLib::RunCommandOnGameThread([&]() {
			UAirBlueprintLib::spawnPlayer(this->simmode_->GetWorld(), success);
		}, true);
		std::this_thread::sleep_for(2s);
	}
	if (!success)
		UE_LOG(LogTemp, Error, TEXT("Could not find valid PlayerStart Position"));

	reset();
}

bool WorldSimApi::destroyObject(const std::string& object_name)
{
	bool result{ false };
	UAirBlueprintLib::RunCommandOnGameThread([this, &object_name, &result]() {
		AActor* actor = UAirBlueprintLib::FindActor<AActor>(simmode_, FString(object_name.c_str()));
		if (actor)
		{
			actor->Destroy();
			result = actor->IsPendingKill();
		}
	}, true);
	return result;
}

std::string WorldSimApi::spawnObject(std::string& object_name, const std::string& load_object, const WorldSimApi::Pose& pose, const WorldSimApi::Vector3r& scale)
{
	// Create struct for Location and Rotation of actor in Unreal
	FTransform actor_transform = simmode_->getGlobalNedTransform().fromGlobalNed(pose);
	bool found_object;
	UAirBlueprintLib::RunCommandOnGameThread([this, load_object, &object_name, &actor_transform, &found_object, &scale]() {
			// Find mesh in /Game and /AirSim asset registry. When more plugins are added this function will have to change
			UStaticMesh* LoadObject = dynamic_cast<UStaticMesh*>(UAirBlueprintLib::GetMeshFromRegistry(load_object));
			if (LoadObject)
			{
				std::vector<std::string> matching_names = UAirBlueprintLib::ListMatchingActors(simmode_->GetWorld(), ".*"+object_name+".*");
				if (matching_names.size() > 0)
				{
					size_t greatest_num{ 0 }, result{ 0 };
					for (auto match : matching_names)
					{
						std::string number_extension = match.substr(match.find_last_not_of("0123456789") + 1);
						if (number_extension != "")
						{
							result = std::stoi(number_extension);
							greatest_num = greatest_num > result ? greatest_num : result;
						}
					}
					object_name += std::to_string(greatest_num + 1);
				}
				FActorSpawnParameters new_actor_spawn_params;
				new_actor_spawn_params.Name = FName(object_name.c_str());
				this->createNewActor(new_actor_spawn_params, actor_transform, scale, LoadObject);
				found_object  = true;
			}
			else
			{
				found_object = false;
			}
	}, true);

	if (!found_object)
	{
		throw std::invalid_argument(
			"There were no objects with name " + load_object + " found in the Registry");
	}
	return object_name;
}

void WorldSimApi::createNewActor(const FActorSpawnParameters& spawn_params, const FTransform& actor_transform, const Vector3r& scale, UStaticMesh* static_mesh)
{
	AActor* NewActor = simmode_->GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, spawn_params); // new
	UStaticMeshComponent* ObjectComponent = NewObject<UStaticMeshComponent>(NewActor);
	ObjectComponent->SetStaticMesh(static_mesh);
	ObjectComponent->SetRelativeLocation(FVector(0, 0, 0));
    ObjectComponent->SetWorldScale3D(FVector(scale[0], scale[1], scale[2]));
	ObjectComponent->SetHiddenInGame(false, true);
	ObjectComponent->RegisterComponent();
	NewActor->SetRootComponent(ObjectComponent);
	NewActor->SetActorLocationAndRotation(actor_transform.GetLocation(), actor_transform.GetRotation(), false, nullptr, ETeleportType::TeleportPhysics);
}

bool WorldSimApi::isPaused() const
{
    return simmode_->isPaused();
}

void WorldSimApi::reset()
{
    UAirBlueprintLib::RunCommandOnGameThread([this]() {
        simmode_->reset();

        //reset any chars we have
        for (auto& c : chars_)
            c.second->reset();
    }, true);
}

void WorldSimApi::pause(bool is_paused)
{
    simmode_->pause(is_paused);
}

void WorldSimApi::continueForTime(double seconds)
{
    simmode_->continueForTime(seconds);
}

void WorldSimApi::setTimeOfDay(bool is_enabled, const std::string& start_datetime, bool is_start_datetime_dst,
    float celestial_clock_speed, float update_interval_secs, bool move_sun)
{
    simmode_->setTimeOfDay(is_enabled, start_datetime, is_start_datetime_dst,
        celestial_clock_speed, update_interval_secs, move_sun);
}

bool WorldSimApi::setSegmentationObjectID(const std::string& mesh_name, int object_id, bool is_name_regex)
{
    bool success;
    UAirBlueprintLib::RunCommandOnGameThread([mesh_name, object_id, is_name_regex, &success]() {
        success = UAirBlueprintLib::SetMeshStencilID(mesh_name, object_id, is_name_regex);
    }, true);
    return success;
}

int WorldSimApi::getSegmentationObjectID(const std::string& mesh_name) const
{
    int result;
    UAirBlueprintLib::RunCommandOnGameThread([&mesh_name, &result]() {
        result = UAirBlueprintLib::GetMeshStencilID(mesh_name);
    }, true);
    return result;
}

void WorldSimApi::printLogMessage(const std::string& message,
    const std::string& message_param, unsigned char severity)
{
    UAirBlueprintLib::LogMessageString(message, message_param, static_cast<LogDebugLevel>(severity));
}

std::vector<std::string> WorldSimApi::listSceneObjects(const std::string& name_regex) const
{
    std::vector<std::string> result;
    UAirBlueprintLib::RunCommandOnGameThread([this, &name_regex, &result]() {
        result = UAirBlueprintLib::ListMatchingActors(simmode_, name_regex);
    }, true);
    return result;
}


WorldSimApi::Pose WorldSimApi::getObjectPose(const std::string& object_name) const
{
    Pose result;
    UAirBlueprintLib::RunCommandOnGameThread([this, &object_name, &result]() {
        AActor* actor = UAirBlueprintLib::FindActor<AActor>(simmode_, FString(object_name.c_str()));
        result = actor ? simmode_->getGlobalNedTransform().toGlobalNed(FTransform(actor->GetActorRotation(), actor->GetActorLocation()))
            : Pose::nanPose();
    }, true);
    return result;
}

bool WorldSimApi::setObjectPose(const std::string& object_name, const WorldSimApi::Pose& pose, bool teleport)
{
    bool result;
    UAirBlueprintLib::RunCommandOnGameThread([this, &object_name, &pose, teleport, &result]() {
        FTransform actor_transform = simmode_->getGlobalNedTransform().fromGlobalNed(pose);
        AActor* actor = UAirBlueprintLib::FindActor<AActor>(simmode_, FString(object_name.c_str()));
        if (actor) {
            if (teleport) 
                result = actor->SetActorLocationAndRotation(actor_transform.GetLocation(), actor_transform.GetRotation(), false, nullptr, ETeleportType::TeleportPhysics);
            else
                result = actor->SetActorLocationAndRotation(actor_transform.GetLocation(), actor_transform.GetRotation(), true);
        }
        else
            result = false;
    }, true);
    return result;
}

void WorldSimApi::enableWeather(bool enable)
{
    UWeatherLib::setWeatherEnabled(simmode_->GetWorld(), enable);
}
void WorldSimApi::setWeatherParameter(WeatherParameter param, float val)
{
    unsigned char param_n = static_cast<unsigned char>(msr::airlib::Utils::toNumeric<WeatherParameter>(param));
    EWeatherParamScalar param_e = msr::airlib::Utils::toEnum<EWeatherParamScalar>(param_n);

    UWeatherLib::setWeatherParamScalar(simmode_->GetWorld(), param_e, val);
}


//------------------------------------------------- Char APIs -----------------------------------------------------------/

void WorldSimApi::charSetFaceExpression(const std::string& expression_name, float value, const std::string& character_name)
{
    AAirSimCharacter* character = getAirSimCharacter(character_name);
    character->setFaceExpression(expression_name, value);
}

float WorldSimApi::charGetFaceExpression(const std::string& expression_name, const std::string& character_name) const
{
    const AAirSimCharacter* character = getAirSimCharacter(character_name);
    return character->getFaceExpression(expression_name);
}

std::vector<std::string> WorldSimApi::charGetAvailableFaceExpressions()
{
    const AAirSimCharacter* character = getAirSimCharacter("");
    return character->getAvailableFaceExpressions();
}

void WorldSimApi::charSetSkinDarkness(float value, const std::string& character_name)
{
    AAirSimCharacter* character = getAirSimCharacter(character_name);
    character->setSkinDarkness(value);
}

float WorldSimApi::charGetSkinDarkness(const std::string& character_name) const
{
    const AAirSimCharacter* character = getAirSimCharacter(character_name);
    return character->getSkinDarkness();
}

void WorldSimApi::charSetSkinAgeing(float value, const std::string& character_name)
{
    AAirSimCharacter* character = getAirSimCharacter(character_name);
    character->setSkinAgeing(value);
}

float WorldSimApi::charGetSkinAgeing(const std::string& character_name) const
{
    const AAirSimCharacter* character = getAirSimCharacter(character_name);
    return character->getSkinAgeing();
}

void WorldSimApi::charSetHeadRotation(const msr::airlib::Quaternionr& q, const std::string& character_name)
{
    AAirSimCharacter* character = getAirSimCharacter(character_name);
    character->setHeadRotation(q);
}

msr::airlib::Quaternionr WorldSimApi::charGetHeadRotation(const std::string& character_name) const
{
    const AAirSimCharacter* character = getAirSimCharacter(character_name);
    return character->getHeadRotation();
}

void WorldSimApi::charSetBonePose(const std::string& bone_name, const msr::airlib::Pose& pose, const std::string& character_name)
{
    AAirSimCharacter* character = getAirSimCharacter(character_name);
    character->setBonePose(bone_name, pose);
}

msr::airlib::Pose WorldSimApi::charGetBonePose(const std::string& bone_name, const std::string& character_name) const
{
    const AAirSimCharacter* character = getAirSimCharacter(character_name);
    return character->getBonePose(bone_name);
}

void WorldSimApi::charResetBonePose(const std::string& bone_name, const std::string& character_name)
{
    AAirSimCharacter* character = getAirSimCharacter(character_name);
    character->resetBonePose(bone_name);
}

void WorldSimApi::charSetFacePreset(const std::string& preset_name, float value, const std::string& character_name)
{
    AAirSimCharacter* character = getAirSimCharacter(character_name);
    character->setFacePreset(preset_name, value);
}

void WorldSimApi::charSetFacePresets(const std::unordered_map<std::string, float>& presets, const std::string& character_name)
{
    AAirSimCharacter* character = getAirSimCharacter(character_name);
    character->setFacePresets(presets);
}
void WorldSimApi::charSetBonePoses(const std::unordered_map<std::string, msr::airlib::Pose>& poses, const std::string& character_name)
{
    AAirSimCharacter* character = getAirSimCharacter(character_name);
    character->setBonePoses(poses);
}
std::unordered_map<std::string, msr::airlib::Pose> WorldSimApi::charGetBonePoses(const std::vector<std::string>& bone_names, const std::string& character_name) const
{
    const AAirSimCharacter* character = getAirSimCharacter(character_name);
    return character->getBonePoses(bone_names);
}

AAirSimCharacter* WorldSimApi::getAirSimCharacter(const std::string& character_name)
{
    AAirSimCharacter* character = nullptr;
    UAirBlueprintLib::RunCommandOnGameThread([this, &character_name, &character]() {
        if (chars_.size() == 0) { //not found in the cache
            TArray<AActor*> characters;
            UAirBlueprintLib::FindAllActor<AAirSimCharacter>(simmode_, characters);
            for (AActor* actor : characters) {
                character = static_cast<AAirSimCharacter*>(actor);
                chars_[std::string(
                    TCHAR_TO_UTF8(*character->GetName()))] = character;
            }
        }

        if (chars_.size() == 0) {
            throw std::invalid_argument(
                "There were no actors of class ACharactor found in the environment");
        }

        //choose first character if name was blank or find by name
        character = character_name == "" ? chars_.begin()->second
            : common_utils::Utils::findOrDefault(chars_, character_name);

        if (!character) {
            throw std::invalid_argument(common_utils::Utils::stringf(
                "Character with name %s was not found in the environment", character_name.c_str()).c_str());
        }
    }, true);

    return character;
}

const AAirSimCharacter* WorldSimApi::getAirSimCharacter(const std::string& character_name) const
{
    return const_cast<WorldSimApi*>(this)->getAirSimCharacter(character_name);
}
//------------------------------------------------- Char APIs -----------------------------------------------------------/

