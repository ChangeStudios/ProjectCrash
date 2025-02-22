/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official Steamworks Documentation: https://partner.steamgames.com/doc/api/ISteamApps
*/

#include "SteamApps/SteamProApps.h"
#include "SteamApps/SteamAppsAsyncTasks.h"
#include "SteamCoreProPluginPrivatePCH.h"

USteamProApps::USteamProApps()
{
#if WITH_STEAMCORE
	OnDLCInstalledCallback.Register(this, &USteamProApps::OnDLCInstalled);
	OnFileDetailsResultCallback.Register(this, &USteamProApps::OnFileDetailsResult);
	OnNewUrlLaunchParametersCallback.Register(this, &USteamProApps::OnNewUrlLaunchParameters);
	OnTimedTrialStatusCallback.Register(this, &USteamProApps::OnTimedTrialStatus);

	if (IsRunningDedicatedServer())
	{
		OnDLCInstalledCallback.SetGameserverFlag();
		OnFileDetailsResultCallback.SetGameserverFlag();
		OnNewUrlLaunchParametersCallback.SetGameserverFlag();
		OnTimedTrialStatusCallback.SetGameserverFlag();
	}
#endif
}

USteamProApps::~USteamProApps()
{
#if WITH_STEAMCORE
	OnDLCInstalledCallback.Unregister();
	OnFileDetailsResultCallback.Unregister();
	OnNewUrlLaunchParametersCallback.Unregister();
	OnTimedTrialStatusCallback.Unregister();
#endif
}

USteamProApps* USteamProApps::GetSteamApps()
{
#if WITH_STEAMCORE
	return SteamApps() ? ThisClass::StaticClass()->GetDefaultObject<USteamProApps>() : nullptr;
#endif
	return nullptr;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		Steam API Functions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

bool USteamProApps::BIsSubscribed()
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (GetApps())
	{
		bResult = GetApps()->BIsSubscribed();
	}
#endif

	return bResult;
}

bool USteamProApps::BIsLowViolence()
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (GetApps())
	{
		bResult = GetApps()->BIsLowViolence();
	}
#endif

	return bResult;
}

bool USteamProApps::BIsCybercafe()
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (GetApps())
	{
		bResult = GetApps()->BIsCybercafe();
	}
#endif

	return bResult;
}

bool USteamProApps::BIsVACBanned()
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;
	
#if WITH_STEAMCORE
	if (GetApps())
	{
		bResult = GetApps()->BIsVACBanned();
	}
#endif

	return bResult;
}

FString USteamProApps::GetCurrentGameLanguage()
{
	LogSteamCoreVeryVerbose("");

	FString Result;

#if WITH_STEAMCORE
	if (GetApps())
	{
		Result = UTF8_TO_TCHAR(GetApps()->GetCurrentGameLanguage());
	}
#endif

	return Result;
}

FString USteamProApps::GetAvailableGameLanguages()
{
	LogSteamCoreVeryVerbose("");

	FString Result;

#if WITH_STEAMCORE
	if (GetApps())
	{
		Result = UTF8_TO_TCHAR(GetApps()->GetAvailableGameLanguages());
	}
#endif

	return Result;
}

bool USteamProApps::BIsSubscribedApp(int32 AppID)
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (GetApps())
	{
		bResult = GetApps()->BIsSubscribedApp(AppID);
	}
#endif

	return bResult;
}

bool USteamProApps::BIsSubscribedFromFamilySharing()
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (GetApps())
	{
		bResult = GetApps()->BIsSubscribedFromFamilySharing();
	}
#endif

	return bResult;
}

bool USteamProApps::BIsTimedTrial(int32& SecondsAllowed, int32& SecondsPlayed)
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;
	SecondsAllowed = 0;
	SecondsPlayed = 0;

#if WITH_STEAMCORE
	uint32 Allowed = 0;
	uint32 Played = 0;
	bResult = GetApps()->BIsTimedTrial(&Allowed, &Played);

	if (bResult)
	{
		SecondsAllowed = Allowed;
		SecondsPlayed = Played;
	}
#endif

	return bResult;
}

bool USteamProApps::SetDlcContext(int32 AppId)
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	bResult = GetApps()->SetDlcContext(AppId);
#endif

	return bResult;
}

int USteamProApps::GetNumBetas(int32& Available, int32& Private)
{
	LogSteamCoreVeryVerbose("");

	int32 Result = 0;

#if WITH_STEAMCORE
	Result = GetApps()->GetNumBetas(&Available, &Private);
#endif

	return Result;
}

bool USteamProApps::GetBetaInfo(int32 BetaIndex, TArray<ESteamBetaBranchFlags>& Flags, int32& BuildId, FString& BetaName, FString& Description)
{
	LogSteamCoreVeryVerbose(""); 

	bool bResult = false;
	Flags.Empty();

#if WITH_STEAMCORE
	uint32 punFlags;
	uint32 punBuildID;
	int cchBetaName = 0;
	int cchDescription = 0;
	char pchBetaName[256];
	char pchDescription[256];

	bResult = GetApps()->GetBetaInfo(BetaIndex, &punFlags, &punBuildID, pchBetaName, cchBetaName, pchDescription, cchDescription);

	if (bResult)
	{
		if (cchBetaName > 0)
		{
			BetaName = UTF8_TO_TCHAR(pchBetaName);
		}

		if (cchDescription > 0)
		{
			Description = UTF8_TO_TCHAR(pchDescription);
		}
		
		LogSteamCoreVerbose("%d, %d, %d, %d, %s, %s", punFlags, punBuildID, cchBetaName, cchDescription, *BetaName, *Description);

		if (punFlags & EBetaBranchFlags::k_EBetaBranch_Default)
		{
			Flags.Add(ESteamBetaBranchFlags::k_EBetaBranch_Default);
		}

		if (punFlags & EBetaBranchFlags::k_EBetaBranch_Available)
		{
			Flags.Add(ESteamBetaBranchFlags::k_EBetaBranch_Available);
		}

		if (punFlags & EBetaBranchFlags::k_EBetaBranch_Installed)
		{
			Flags.Add(ESteamBetaBranchFlags::k_EBetaBranch_Installed);
		}
	
		if (punFlags & EBetaBranchFlags::k_EBetaBranch_None)
		{
			Flags.Add(ESteamBetaBranchFlags::k_EBetaBranch_None);
		}

		if (punFlags & EBetaBranchFlags::k_EBetaBranch_Private)
		{
			Flags.Add(ESteamBetaBranchFlags::k_EBetaBranch_Private);
		}

		if (punFlags & EBetaBranchFlags::k_EBetaBranch_Selected)
		{
			Flags.Add(ESteamBetaBranchFlags::k_EBetaBranch_Selected);
		}
	}
#endif

	return bResult;
}

bool USteamProApps::SetActiveBeta(FString BetaName)
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	bResult = GetApps()->SetActiveBeta(TCHAR_TO_UTF8(*BetaName));
#endif

	return bResult;
}

bool USteamProApps::BIsDlcInstalled(int32 AppID)
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (GetApps())
	{
		bResult = GetApps()->BIsDlcInstalled(AppID);
	}
#endif

	return bResult;
}

int32 USteamProApps::GetEarliestPurchaseUnixTime(int32 AppID)
{
	LogSteamCoreVeryVerbose("");

	int32 Result = 0;

#if WITH_STEAMCORE
	if (GetApps())
	{
		Result = GetApps()->GetEarliestPurchaseUnixTime(AppID);
	}
#endif

	return Result;
}

bool USteamProApps::BIsSubscribedFromFreeWeekend()
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (GetApps())
	{
		bResult = GetApps()->BIsSubscribedFromFreeWeekend();
	}
#endif

	return bResult;
}

int32 USteamProApps::GetDLCCount()
{
	LogSteamCoreVeryVerbose("");

	int32 Result = 0;

#if WITH_STEAMCORE
	if (GetApps())
	{
		Result = GetApps()->GetDLCCount();
	}
#endif

	return Result;
}

bool USteamProApps::BGetDLCDataByIndex(int32 DLC, int32& OutAppId, bool& bAvailable, FString& OutName)
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;
	OutAppId = 0;
	bAvailable = false;
	OutName.Empty();

#if WITH_STEAMCORE
	if (GetApps())
	{
		TArray<char> DataArray;
		DataArray.SetNum(1024);
		AppId_t AppId;

		bResult = GetApps()->BGetDLCDataByIndex(DLC, &AppId, &bAvailable, DataArray.GetData(), DataArray.Num());

		if (bResult)
		{
			OutName = UTF8_TO_TCHAR(DataArray.GetData());
			OutAppId = AppId;
		}
	}
#endif

	return bResult;
}

void USteamProApps::InstallDLC(int32 AppID)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (GetApps())
	{
		GetApps()->InstallDLC(AppID);
	}
#endif
}

void USteamProApps::UninstallDLC(int32 AppID)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (GetApps())
	{
		GetApps()->UninstallDLC(AppID);
	}
#endif
}

bool USteamProApps::GetCurrentBetaName(FString& OutName)
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;
	OutName.Empty();

#if WITH_STEAMCORE
	if (GetApps())
	{
		TArray<char> DataArray;
		DataArray.SetNum(512);

		bResult = GetApps()->GetCurrentBetaName(DataArray.GetData(), DataArray.Num());

		if (bResult)
		{
			OutName = UTF8_TO_TCHAR(DataArray.GetData());
		}
	}
#endif

	return bResult;
}

bool USteamProApps::MarkContentCorrupt(bool bMissingFilesOnly)
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (GetApps())
	{
		bResult = GetApps()->MarkContentCorrupt(bMissingFilesOnly);
	}
#endif

	return bResult;
}

int32 USteamProApps::GetInstalledDepots(int32 AppID, int32 MaxDepots, TArray<int32>& OutDepots)
{
	LogSteamCoreVeryVerbose("");

	int32 Result = 0;
	OutDepots.Empty();

#if WITH_STEAMCORE
	if (GetApps())
	{
		TArray<DepotId_t> DataArray;
		DataArray.SetNum(MaxDepots);

		Result = GetApps()->GetInstalledDepots(AppID, DataArray.GetData(), DataArray.Num());

		if (Result > 0)
		{
			for (int32 i = 0; i < DataArray.Num(); i++)
			{
				OutDepots.Add(DataArray[i]);
			}
		}
	}
#endif

	return Result;
}

int USteamProApps::GetLaunchCommandLine(FString& OutCommandline)
{
	LogSteamCoreVeryVerbose("");

	int Result = 0;
	OutCommandline.Empty();

#if WITH_STEAMCORE
	if (GetApps())
	{
		TArray<char> DataArray;
		DataArray.SetNum(1024);

		Result = GetApps()->GetLaunchCommandLine(DataArray.GetData(), DataArray.Num());

		OutCommandline = UTF8_TO_TCHAR(DataArray.GetData());
	}
#endif

	return Result;
}

int32 USteamProApps::GetAppInstallDir(int32 AppID, FString& OutFolder)
{
	LogSteamCoreVeryVerbose("");

	int32 Result = 0;
	OutFolder.Empty();

#if WITH_STEAMCORE
	if (GetApps())
	{
		TArray<char> DataArray;
		DataArray.SetNum(512);

		Result = GetApps()->GetAppInstallDir(AppID, DataArray.GetData(), DataArray.Num());

		if (Result > 0)
		{
			OutFolder = UTF8_TO_TCHAR(DataArray.GetData());
		}
	}
#endif

	return Result;
}

bool USteamProApps::BIsAppInstalled(int32 AppID)
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (GetApps())
	{
		bResult = GetApps()->BIsAppInstalled(AppID);
	}
#endif

	return bResult;
}

FSteamID USteamProApps::GetAppOwner()
{
	LogSteamCoreVeryVerbose("");

	FSteamID Result;

#if WITH_STEAMCORE
	if (GetApps())
	{
		Result = GetApps()->GetAppOwner();
	}
#endif

	return Result;
}

FString USteamProApps::GetLaunchQueryParam(FString Key)
{
	LogSteamCoreVeryVerbose("");

	FString Result;

	if (Key.Len() == 0)
	{
		return Result;
	}

#if WITH_STEAMCORE
	if (GetApps())
	{
		const FTCHARToUTF8 Converter(*Key);
		Result = FString(UTF8_TO_TCHAR(GetApps()->GetLaunchQueryParam(Converter.Get())));
	}
#endif

	return Result;
}

bool USteamProApps::GetDlcDownloadProgress(int32 AppID, int32& OutBytesDownloaded, int32& OutBytesTotal)
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;
	uint64 BytesDownloaded = 0;
	uint64 BytesTotal = 0;

#if WITH_STEAMCORE
	if (GetApps())
	{
		bResult = GetApps()->GetDlcDownloadProgress(AppID, &BytesDownloaded, &BytesTotal);
	}
#endif

	OutBytesDownloaded = BytesDownloaded;
	OutBytesTotal = BytesTotal;

	return bResult;
}

int32 USteamProApps::GetAppBuildId()
{
	LogSteamCoreVeryVerbose("");

	int32 Result = 0;

#if WITH_STEAMCORE
	if (GetApps())
	{
		Result = GetApps()->GetAppBuildId();
	}
#endif

	return Result;
}

void USteamProApps::GetFileDetails(const FOnFileDetailsResult& Callback, FString FileName)
{
	LogSteamCoreVeryVerbose("");

#if WITH_STEAMCORE
	if (GetApps())
	{
		FOnlineAsyncTaskSteamCoreProAppsGetFileDetails* Task = new FOnlineAsyncTaskSteamCoreProAppsGetFileDetails(Callback, FileName);
		QueueAsyncTask(Task);
	}
#endif
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		Steam API Callbacks
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

#if WITH_STEAMCORE
void USteamProApps::OnDLCInstalled(DlcInstalled_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		DLCInstalled.Broadcast(Data);
	});

}

void USteamProApps::OnFileDetailsResult(FileDetailsResult_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		FileDetailsResultDelegate.Broadcast(Data);
	});
}

void USteamProApps::OnNewUrlLaunchParameters(NewUrlLaunchParameters_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		NewUrlLaunchParametersDelegate.Broadcast();
	});
}

void USteamProApps::OnTimedTrialStatus(TimedTrialStatus_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		TimedTrialStatusDelegate.Broadcast(Data);
	});
}
#endif
