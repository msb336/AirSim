#pragma once
#include "CoreMinimal.h"
#include "Runtime/PakFile/Public/IPlatformFilePak.h"
#include "Runtime/Core/Public/HAL/PlatformFilemanager.h"

class PakLoader
{
public:
	PakLoader();
	~PakLoader();
	void initialize();


private:
	FPakPlatformFile* PakPlatform = new FPakPlatformFile();
};