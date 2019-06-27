#pragma once

#include "AirsimLevelStreaming.h"

UAirsimLevelStreaming* UAirsimLevelStreaming::LoadAirsimLevelInstance(UObject* WorldContextObject, FString LevelName, FVector Location, FRotator Rotation, bool& bOutSuccess)
{
	auto level_pointer =  dynamic_cast<UAirsimLevelStreaming*> (LoadLevelInstance(WorldContextObject, LevelName, Location, Rotation, bOutSuccess));
	return level_pointer;
}