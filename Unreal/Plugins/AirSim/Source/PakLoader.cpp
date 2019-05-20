#include "PakLoader.h"



PakLoader::PakLoader()
{
	
}


PakLoader::~PakLoader()
{
}

void PakLoader::initialize()
{

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	FPakPlatformFile* PakPlatform = new FPakPlatformFile();
	PakPlatform->Initialize(&PlatformFile, TEXT(""));
	FPlatformFileManager::Get().SetPlatformFile(*PakPlatform);
	FString PakPath = TEXT("Building_99.pak");
	UE_LOG(LogTemp, Log, TEXT("Mounting pak %s"), *PakPath);
	if (!PakPlatform->Mount(*PakPath, 0, TEXT("C:\\Users\\v-mattbr\\repos\\unreal-envs\\AirSimExe\\DlcMaps\\"))) {
		UE_LOG(LogTemp, Error, TEXT("Failed to mount %s"), *PakPath);
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("found pakfile"));
		//FArrayReader SerializedAssetData;
		//int32 DashPosition = PakFilename.Find(TEXT("-"), ESearchCase::CaseSensitive, ESearchDir::FromEnd);
		//if (DashPosition != -1)
		//{
		//	PakFilename = PakFilename.Left(DashPosition);
		//	FString AssetRegistryName = PakFilename + TEXT("-AssetRegistry.bin");
		//	UE_LOG(LogTemp, Log, TEXT("AssetRegistryName %s"), *AssetRegistryName);
		//	if (FFileHelper::LoadFileToArray(SerializedAssetData, *(FPaths::GameDir() / AssetRegistryName)))
		//	{
		//		// serialize the data with the memory reader (will convert FStrings to FNames, etc)
		//		AssetRegistryModule.Get().Serialize(SerializedAssetData);
		//	}
		//	else
		//	{
		//		UE_LOG(LogTemp, Warning, TEXT("%s could not be found"), *AssetRegistryName);
		//	}
		//}
	}
}
