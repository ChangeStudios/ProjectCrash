/**
* Copyright (C) 2017-2024 eelDev AB
*/

#include "SteamGameSearch/SteamProGameSearch.h"
#include "SteamCoreProPluginPrivatePCH.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		GameSearch API Functions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

USteamProGameSearch::USteamProGameSearch()
{
#if WITH_STEAMCORE
	if (SteamGameSearch())
	{
		OnSearchForGameProgressCallback.Register(this, &USteamProGameSearch::OnSearchForGameProgressUpdated);
		OnSearchForGameResultCallback.Register(this, &USteamProGameSearch::OnSearchForGameResultUpdated);
		OnRequestPlayersForGameProgressCallback.Register(this, &USteamProGameSearch::OnRequestPlayersForGameProgressUpdated);
		OnRequestPlayersForGameResultCallback.Register(this, &USteamProGameSearch::OnRequestPlayersForGameResultUpdated);
		OnRequestPlayersForGameFinalResultCallback.Register(this, &USteamProGameSearch::OnRequestPlayersForGameFinalResultUpdated);
		OnSubmitPlayerResultResultCallback.Register(this, &USteamProGameSearch::OnSubmitPlayerResultResultUpdated);
		OnEndGameResultCallback.Register(this, &USteamProGameSearch::OnEndGameResultUpdated);

		if (IsRunningDedicatedServer())
		{
			OnSearchForGameProgressCallback.SetGameserverFlag();
			OnSearchForGameResultCallback.SetGameserverFlag();
			OnRequestPlayersForGameProgressCallback.SetGameserverFlag();
			OnRequestPlayersForGameResultCallback.SetGameserverFlag();
			OnRequestPlayersForGameFinalResultCallback.SetGameserverFlag();
			OnSubmitPlayerResultResultCallback.SetGameserverFlag();
			OnEndGameResultCallback.SetGameserverFlag();
		}
	}
#endif
}

USteamProGameSearch::~USteamProGameSearch()
{
#if WITH_STEAMCORE
	if (SteamGameSearch())
	{
		OnSearchForGameProgressCallback.Unregister();
		OnSearchForGameResultCallback.Unregister();
		OnRequestPlayersForGameProgressCallback.Unregister();
		OnRequestPlayersForGameResultCallback.Unregister();
		OnRequestPlayersForGameFinalResultCallback.Unregister();
		OnSubmitPlayerResultResultCallback.Unregister();
		OnEndGameResultCallback.Unregister();
	}
#endif
}

USteamProGameSearch* USteamProGameSearch::GetSteamGameSearch()
{
#if WITH_STEAMCORE
	return SteamGameSearch() ? ThisClass::StaticClass()->GetDefaultObject<USteamProGameSearch>() : nullptr;
#else
	return nullptr;
#endif
}

ESteamGameSearchErrorCode USteamProGameSearch::AddGameSearchParams(FString KeyToFind, TArray<FString> ValuesToFind)
{
	LogSteamCoreVerbose("");
	
#if WITH_STEAMCORE
	EGameSearchErrorCode_t Result = k_EGameSearchErrorCode_Failed_Unknown_Error;

	if (SteamGameSearch())
	{
		TArray<char> Values;
		Values.SetNum(ValuesToFind.Num());

		for (int32 i = 0; i < ValuesToFind.Num(); i++)
		{
			if (i > 0)
			{
				Values[i] = *TCHAR_TO_UTF8(*FString::Printf(TEXT(",%s"), *ValuesToFind[i]));
			}
			else
			{
				Values[i] = *TCHAR_TO_UTF8(*FString::Printf(TEXT("%s"), *ValuesToFind[i]));
			}
		}

		Result = SteamGameSearch()->AddGameSearchParams(TCHAR_TO_UTF8(*KeyToFind), Values.GetData());
	}

	return static_cast<ESteamGameSearchErrorCode>(Result);
#endif

	return ESteamGameSearchErrorCode::Invalid;
}

ESteamGameSearchErrorCode USteamProGameSearch::SearchForGameWithLobby(FSteamID SteamIDLobby, int32 PlayerMin, int32 PlayerMax)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	EGameSearchErrorCode_t Result = k_EGameSearchErrorCode_Failed_Unknown_Error;

	if (SteamGameSearch())
	{
		Result = SteamGameSearch()->SearchForGameWithLobby(SteamIDLobby, PlayerMin, PlayerMax);
	}

	return static_cast<ESteamGameSearchErrorCode>(Result);
#else
	return ESteamGameSearchErrorCode::Invalid;
#endif
}

ESteamGameSearchErrorCode USteamProGameSearch::SearchForGameSolo(int32 PlayerMin, int32 PlayerMax)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	EGameSearchErrorCode_t Result = k_EGameSearchErrorCode_Failed_Unknown_Error;

	if (SteamGameSearch())
	{
		Result = SteamGameSearch()->SearchForGameSolo(PlayerMin, PlayerMax);
	}

	return static_cast<ESteamGameSearchErrorCode>(Result);

#else
	return ESteamGameSearchErrorCode::Invalid;
#endif

}

ESteamGameSearchErrorCode USteamProGameSearch::AcceptGame()
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	EGameSearchErrorCode_t Result = k_EGameSearchErrorCode_Failed_Unknown_Error;

	if (SteamGameSearch())
	{
		Result = SteamGameSearch()->AcceptGame();
	}

	return static_cast<ESteamGameSearchErrorCode>(Result);
#else
	return ESteamGameSearchErrorCode::Invalid;
#endif
}

ESteamGameSearchErrorCode USteamProGameSearch::DeclineGame()
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	EGameSearchErrorCode_t Result = k_EGameSearchErrorCode_Failed_Unknown_Error;

	if (SteamGameSearch())
	{
		Result = SteamGameSearch()->DeclineGame();
	}

	return static_cast<ESteamGameSearchErrorCode>(Result);
#else
	return ESteamGameSearchErrorCode::Invalid;
#endif
}

ESteamGameSearchErrorCode USteamProGameSearch::RetrieveConnectionDetails(FSteamID SteamIDHost, FString& ConnectionDetails, int32 NumConnectionDetails)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	EGameSearchErrorCode_t Result = k_EGameSearchErrorCode_Failed_Unknown_Error;

	if (SteamGameSearch())
	{
		char* ConnectionDetailsChar = nullptr;
		Result = SteamGameSearch()->RetrieveConnectionDetails(SteamIDHost, ConnectionDetailsChar, NumConnectionDetails);

		if (Result == k_EGameSearchErrorCode_OK)
		{
			ConnectionDetails = FString(ConnectionDetailsChar);
		}
	}

	return static_cast<ESteamGameSearchErrorCode>(Result);
#else
	return ESteamGameSearchErrorCode::Invalid;
#endif
}

ESteamGameSearchErrorCode USteamProGameSearch::EndGameSearch()
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	EGameSearchErrorCode_t Result = k_EGameSearchErrorCode_Failed_Unknown_Error;

	if (SteamGameSearch())
	{
		Result = SteamGameSearch()->EndGameSearch();
	}

	return static_cast<ESteamGameSearchErrorCode>(Result);

#else
	return ESteamGameSearchErrorCode::Invalid;
#endif
}

ESteamGameSearchErrorCode USteamProGameSearch::SetGameHostParams(FString Key, TArray<FString> Values)
{
#if WITH_STEAMCORE
	EGameSearchErrorCode_t Result = k_EGameSearchErrorCode_Failed_Unknown_Error;

	if (SteamGameSearch())
	{
		TArray<char> CharValues;
		CharValues.SetNum(Values.Num());

		for (int32 i = 0; i < Values.Num(); i++)
		{
			if (i > 0)
			{
				CharValues[i] = *TCHAR_TO_UTF8(*FString::Printf(TEXT(",%s"), *Values[i]));
			}
			else
			{
				CharValues[i] = *TCHAR_TO_UTF8(*FString::Printf(TEXT("%s"), *Values[i]));
			}
		}

		Result = SteamGameSearch()->SetGameHostParams(TCHAR_TO_UTF8(*Key), CharValues.GetData());
	}

	return static_cast<ESteamGameSearchErrorCode>(Result);
#endif

	return ESteamGameSearchErrorCode::Invalid;
}

ESteamGameSearchErrorCode USteamProGameSearch::SetConnectionDetails(FString ConnectionDetails)
{
#if WITH_STEAMCORE
	EGameSearchErrorCode_t Result = k_EGameSearchErrorCode_Failed_Unknown_Error;

	if (SteamGameSearch())
	{
		Result = SteamGameSearch()->SetConnectionDetails(TCHAR_TO_UTF8(*ConnectionDetails), 1);
	}

	return static_cast<ESteamGameSearchErrorCode>(Result);
#endif

	return ESteamGameSearchErrorCode::Invalid;
}

ESteamGameSearchErrorCode USteamProGameSearch::RequestPlayersForGame(int32 PlayerMin, int32 PlayerMax, int32 MaxTeamSize)
{
#if WITH_STEAMCORE
	EGameSearchErrorCode_t Result = k_EGameSearchErrorCode_Failed_Unknown_Error;

	if (SteamGameSearch())
	{
		Result = SteamGameSearch()->RequestPlayersForGame(PlayerMin, PlayerMax, MaxTeamSize);
	}

	return static_cast<ESteamGameSearchErrorCode>(Result);

#else
	return ESteamGameSearchErrorCode::Invalid;
#endif

}

ESteamGameSearchErrorCode USteamProGameSearch::HostConfirmGameStart(FString UniqueGameID)
{
#if WITH_STEAMCORE
	EGameSearchErrorCode_t Result = k_EGameSearchErrorCode_Failed_Unknown_Error;

	if (SteamGameSearch())
	{
		Result = SteamGameSearch()->HostConfirmGameStart(FCString::Atoi64(*UniqueGameID));
	}

	return static_cast<ESteamGameSearchErrorCode>(Result);
#endif

	return ESteamGameSearchErrorCode::Invalid;
}

ESteamGameSearchErrorCode USteamProGameSearch::CancelRequestPlayersForGame()
{
#if WITH_STEAMCORE
	EGameSearchErrorCode_t Result = k_EGameSearchErrorCode_Failed_Unknown_Error;

	if (SteamGameSearch())
	{
		Result = SteamGameSearch()->CancelRequestPlayersForGame();
	}

	return static_cast<ESteamGameSearchErrorCode>(Result);

#else
	return ESteamGameSearchErrorCode::Invalid;
#endif

}

ESteamGameSearchErrorCode USteamProGameSearch::SubmitPlayerResult(FString UniqueGameID, FSteamID SteamIDPlayer, ESteamPlayerResult PlayerResult)
{
#if WITH_STEAMCORE
	EGameSearchErrorCode_t Result = k_EGameSearchErrorCode_Failed_Unknown_Error;

	if (SteamGameSearch())
	{
		Result = SteamGameSearch()->SubmitPlayerResult(FCString::Atoi64(*UniqueGameID), SteamIDPlayer, static_cast<EPlayerResult_t>(PlayerResult));
	}

	return static_cast<ESteamGameSearchErrorCode>(Result);
#endif

	return ESteamGameSearchErrorCode::Invalid;
}

ESteamGameSearchErrorCode USteamProGameSearch::EndGame(FString UniqueGameID)
{
#if WITH_STEAMCORE
	EGameSearchErrorCode_t Result = k_EGameSearchErrorCode_Failed_Unknown_Error;

	if (SteamGameSearch())
	{
		Result = SteamGameSearch()->EndGame(FCString::Atoi64(*UniqueGameID));
	}

	return static_cast<ESteamGameSearchErrorCode>(Result);
#endif

	return ESteamGameSearchErrorCode::Invalid;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		Steam API Callbacks
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

#if WITH_STEAMCORE

void USteamProGameSearch::OnEndGameResultUpdated(EndGameResultCallback_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		EndGameResultDelegate.Broadcast(Data);
	});
}

void USteamProGameSearch::OnSubmitPlayerResultResultUpdated(SubmitPlayerResultResultCallback_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		SubmitPlayerResultResultDelegate.Broadcast(Data);
	});
}

void USteamProGameSearch::OnRequestPlayersForGameFinalResultUpdated(RequestPlayersForGameFinalResultCallback_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		RequestPlayersForGameFinalResultDelegate.Broadcast(Data);
	});
}

void USteamProGameSearch::OnRequestPlayersForGameResultUpdated(RequestPlayersForGameResultCallback_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		RequestPlayersForGameResultDelegate.Broadcast(Data);
	});
}

void USteamProGameSearch::OnRequestPlayersForGameProgressUpdated(RequestPlayersForGameProgressCallback_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		RequestPlayersForGameProgressDelegate.Broadcast(Data);
	});
}

void USteamProGameSearch::OnSearchForGameResultUpdated(SearchForGameResultCallback_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		SearchForGameResultDelegate.Broadcast(Data);
	});
}

void USteamProGameSearch::OnSearchForGameProgressUpdated(SearchForGameProgressCallback_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		SearchForGameProgressDelegate.Broadcast(Data);
	});
}
#endif