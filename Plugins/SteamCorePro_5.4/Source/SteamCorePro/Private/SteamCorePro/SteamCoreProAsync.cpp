/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official SteamCorePro Documentation: https://eeldev.com
*/

#include "SteamCorePro/SteamCoreProAsync.h"
#include "SteamCoreProPluginPrivatePCH.h"
#include "SteamCoreProModule.h"
#include "SteamCorePro/SteamTypes.h"

void USteamCoreProAsyncAction::Activate()
{
	Super::Activate();
}

void USteamCoreProAsyncAction::SetReadyToDestroy()
{
	Super::SetReadyToDestroy();
}

#if WITH_STEAMCORE
FOnlineAsyncTaskSteamCorePro::FOnlineAsyncTaskSteamCorePro()
	: FOnlineAsyncTaskBasic(nullptr)
	, bInit(false)
	, bTimedOut(false)
	, m_CallbackHandle(k_uAPICallInvalid)
	, m_AsyncTimeout(10.f)
{
}

void FOnlineAsyncTaskSteamCorePro::Tick()
{
	LogSteamCoreVerbose("Tick: %s", *ToString());

	if (!bIsComplete)
	{
		bTimedOut = GetElapsedTime() > m_AsyncTimeout;

		if (bTimedOut)
		{
			bIsComplete = true;
			bWasSuccessful = false;

			LogSteamCoreVerbose("%s timed out or finished ", *ToString());
		}
	}
}
#endif
