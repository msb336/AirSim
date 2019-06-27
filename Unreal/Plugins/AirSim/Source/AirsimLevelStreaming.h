#pragma once
#include "Runtime/Engine/Classes/Engine/LevelStreamingDynamic.h"
//#include "AirsimLevelStreaming.generated.h"


class UAirsimLevelStreaming : public ULevelStreamingDynamic
{
public:
	static UAirsimLevelStreaming* LoadAirsimLevelInstance(UObject* WorldContextObject, FString LevelName, FVector Location, FRotator Rotation, bool& bOutSuccess);
	//void trigger();
};