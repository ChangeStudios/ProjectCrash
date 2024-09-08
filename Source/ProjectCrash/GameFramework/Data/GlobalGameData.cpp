// Copyright Samuel Reitich. All rights reserved.


#include "GameFramework/Data/GlobalGameData.h"

#include "GameFramework/CrashAssetManager.h"

UGlobalGameData::UGlobalGameData()
{
}

const UGlobalGameData& UGlobalGameData::Get()
{
	return UCrashAssetManager::Get().GetGlobalGameData();
}
