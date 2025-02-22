/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official Steamworks Documentation: https://partner.steamgames.com/doc/api/ISteamFriends
*/

#include "SteamFriends/SteamFriendsAsyncActions.h"
#include "SteamFriends/SteamFriendsAsyncTasks.h"
#include "SteamCoreProPluginPrivatePCH.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		USteamCoreProFriendsAsyncActionSetPersonaName
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
USteamCoreProFriendsAsyncActionSetPersonaName* USteamCoreProFriendsAsyncActionSetPersonaName::SetPersonaNameAsync(UObject* WorldContextObject, FString Name, float Timeout)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (USteamCoreProSubsystem* Subsystem = USteamCoreProSubsystem::Get())
	{
		const auto AsyncObject = NewObject<USteamCoreProFriendsAsyncActionSetPersonaName>();
		const auto Task = new FOnlineAsyncTaskSteamCoreProFriendsSetPersonaName(AsyncObject, Name, Timeout);
		AsyncObject->RegisterWithGameInstance(WorldContextObject);
		
		Subsystem->QueueAsyncTask(Task);
		AsyncObject->Activate();

		return AsyncObject;
	}
#endif

	return nullptr;
}

void USteamCoreProFriendsAsyncActionSetPersonaName::HandleCallback(const FSetPersonaNameResponse& Data, bool bWasSuccessful)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	AsyncTask(ENamedThreads::GameThread, [this, Data, bWasSuccessful]()
	{
		OnCallback.Broadcast(Data, bWasSuccessful);

		SetReadyToDestroy();
	});
#endif
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		USteamCoreProFriendsAsyncActionDownloadClanActivityCounts
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
USteamCoreProFriendsAsyncActionDownloadClanActivityCounts* USteamCoreProFriendsAsyncActionDownloadClanActivityCounts::DownloadClanActivityCountsAsync(UObject* WorldContextObject, TArray<FSteamID> SteamIDClans, float Timeout)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (USteamCoreProSubsystem* Subsystem = USteamCoreProSubsystem::Get())
	{
		const auto AsyncObject = NewObject<USteamCoreProFriendsAsyncActionDownloadClanActivityCounts>();
		const auto Task = new FOnlineAsyncTaskSteamCoreProFriendsDownloadClanActivityCounts(AsyncObject, SteamIDClans, Timeout);
		AsyncObject->RegisterWithGameInstance(WorldContextObject);
		
		Subsystem->QueueAsyncTask(Task);

		AsyncObject->Activate();

		return AsyncObject;
	}
#endif
	
	return nullptr;
}

void USteamCoreProFriendsAsyncActionDownloadClanActivityCounts::HandleCallback(const FDownloadClanActivityCountsResult& Data, bool bWasSuccessful)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	AsyncTask(ENamedThreads::GameThread, [this, Data, bWasSuccessful]()
	{
		OnCallback.Broadcast(Data, bWasSuccessful);

		SetReadyToDestroy();
	});
#endif
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		USteamCoreProFriendsAsyncActionRequestClanOfficerList
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
USteamCoreProFriendsAsyncActionRequestClanOfficerList* USteamCoreProFriendsAsyncActionRequestClanOfficerList::RequestClanOfficerListAsync(UObject* WorldContextObject, FSteamID SteamIDClan, float Timeout)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (USteamCoreProSubsystem* Subsystem = USteamCoreProSubsystem::Get())
	{

		const auto AsyncObject = NewObject<USteamCoreProFriendsAsyncActionRequestClanOfficerList>();
		const auto Task = new FOnlineAsyncTaskSteamCoreProRequestClanOfficerList(AsyncObject, SteamIDClan, Timeout);
		AsyncObject->RegisterWithGameInstance(WorldContextObject);
		
		Subsystem->QueueAsyncTask(Task);
		AsyncObject->Activate();

		return AsyncObject;
	}
#endif

	return nullptr;
}

void USteamCoreProFriendsAsyncActionRequestClanOfficerList::HandleCallback(const FClanOfficerListResponse& Data, bool bWasSuccessful)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	AsyncTask(ENamedThreads::GameThread, [this, Data, bWasSuccessful]()
	{
		OnCallback.Broadcast(Data, bWasSuccessful);

		SetReadyToDestroy();
	});
#endif
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		USteamCoreProFriendsAsyncActionJoinClanChatRoom
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
USteamCoreProFriendsAsyncActionJoinClanChatRoom* USteamCoreProFriendsAsyncActionJoinClanChatRoom::JoinClanChatRoomAsync(UObject* WorldContextObject, FSteamID SteamIDClan, float Timeout)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (USteamCoreProSubsystem* Subsystem = USteamCoreProSubsystem::Get())
	{
		const auto AsyncObject = NewObject<USteamCoreProFriendsAsyncActionJoinClanChatRoom>();
		const auto Task = new FOnlineAsyncTaskSteamCoreProFriendsJoinClanChatRoom(AsyncObject, SteamIDClan, Timeout);
		AsyncObject->RegisterWithGameInstance(WorldContextObject);
		
		Subsystem->QueueAsyncTask(Task);
		AsyncObject->Activate();

		return AsyncObject;
	}
#endif
	
	return nullptr;
}

void USteamCoreProFriendsAsyncActionJoinClanChatRoom::HandleCallback(const FJoinClanChatRoomCompletionResult& Data, bool bWasSuccessful)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	AsyncTask(ENamedThreads::GameThread, [this, Data, bWasSuccessful]()
	{
		OnCallback.Broadcast(Data, bWasSuccessful);

		SetReadyToDestroy();
	});
#endif
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		USteamCoreProFriendsAsyncActionEnumerateFollowingList
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
USteamCoreProFriendsAsyncActionEnumerateFollowingList* USteamCoreProFriendsAsyncActionEnumerateFollowingList::EnumerateFollowingListAsync(UObject* WorldContextObject, int32 StartIndex, float Timeout)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (USteamCoreProSubsystem* Subsystem = USteamCoreProSubsystem::Get())
	{
		const auto AsyncObject = NewObject<USteamCoreProFriendsAsyncActionEnumerateFollowingList>();
		const auto Task = new FOnlineAsyncTaskSteamCoreProFriendsEnumerateFollowingList(AsyncObject, StartIndex, Timeout);
		AsyncObject->RegisterWithGameInstance(WorldContextObject);
		
		Subsystem->QueueAsyncTask(Task);
		AsyncObject->Activate();

		return AsyncObject;
	}
#endif

	return nullptr;
}

void USteamCoreProFriendsAsyncActionEnumerateFollowingList::HandleCallback(const FFriendsEnumerateFollowingList& Data, bool bWasSuccessful)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	AsyncTask(ENamedThreads::GameThread, [this, Data, bWasSuccessful]()
	{
		OnCallback.Broadcast(Data, bWasSuccessful);

		SetReadyToDestroy();
	});
#endif
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		USteamCoreProFriendsAsyncActionIsFollowing
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
USteamCoreProFriendsAsyncActionIsFollowing* USteamCoreProFriendsAsyncActionIsFollowing::IsFollowingAsync(UObject* WorldContextObject, FSteamID SteamID, float Timeout)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (USteamCoreProSubsystem* Subsystem = USteamCoreProSubsystem::Get())
	{
		const auto AsyncObject = NewObject<USteamCoreProFriendsAsyncActionIsFollowing>();
		const auto Task = new FOnlineAsyncTaskSteamCoreProFriendsIsFollowing(AsyncObject, SteamID, Timeout);
		AsyncObject->RegisterWithGameInstance(WorldContextObject);
		
		Subsystem->QueueAsyncTask(Task);
		AsyncObject->Activate();

		return AsyncObject;
	}
#endif

	return nullptr;
}

void USteamCoreProFriendsAsyncActionIsFollowing::HandleCallback(const FFriendsIsFollowing& Data, bool bWasSuccessful)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	AsyncTask(ENamedThreads::GameThread, [this, Data, bWasSuccessful]()
	{
		OnCallback.Broadcast(Data, bWasSuccessful);

		SetReadyToDestroy();
	});
#endif
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		USteamCoreProFriendsAsyncActionIsFollowing
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
USteamCoreProFriendsAsyncActionGetFollowerCount* USteamCoreProFriendsAsyncActionGetFollowerCount::GetFollowerCountAsync(UObject* WorldContextObject, FSteamID SteamID, float Timeout)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (USteamCoreProSubsystem* Subsystem = USteamCoreProSubsystem::Get())
	{
		const auto AsyncObject = NewObject<USteamCoreProFriendsAsyncActionGetFollowerCount>();
		const auto Task = new FOnlineAsyncTaskSteamCoreProFriendsGetFollowerCount(AsyncObject, SteamID, Timeout);
		AsyncObject->RegisterWithGameInstance(WorldContextObject);
		
		Subsystem->QueueAsyncTask(Task);
		AsyncObject->Activate();

		return AsyncObject;
	}
#endif

	return nullptr;
}

void USteamCoreProFriendsAsyncActionGetFollowerCount::HandleCallback(const FFriendsGetFollowerCount& Data, bool bWasSuccessful)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	AsyncTask(ENamedThreads::GameThread, [this, Data, bWasSuccessful]()
	{
		OnCallback.Broadcast(Data, bWasSuccessful);

		SetReadyToDestroy();
	});
#endif
}

USteamCoreProFriendsAsyncActionRequestUserInformation* USteamCoreProFriendsAsyncActionRequestUserInformation::RequestUserInformationAsync(UObject* WorldContextObject, FSteamID SteamIDUser, bool bRequireNameOnly, float Timeout)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (USteamCoreProSubsystem* Subsystem = USteamCoreProSubsystem::Get())
	{
		const auto AsyncObject = NewObject<USteamCoreProFriendsAsyncActionRequestUserInformation>();
		AsyncObject->RegisterWithGameInstance(WorldContextObject);
		
		AsyncObject->OnPersonaStateChangeCallback.Register(AsyncObject, &USteamCoreProFriendsAsyncActionRequestUserInformation::OnPersonaStateChange);
		AsyncObject->m_SteamIDUser = SteamIDUser;
		AsyncObject->m_bRequireNameOnly = bRequireNameOnly;
		AsyncObject->Activate();

		return AsyncObject;
	}
#endif

	return nullptr;
}

void USteamCoreProFriendsAsyncActionRequestUserInformation::Activate()
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	USteamCoreProAsyncAction::Activate();

	if (!SteamFriends()->RequestUserInformation(m_SteamIDUser, m_bRequireNameOnly))
	{
		HandleCallback();
	}
#endif
}

void USteamCoreProFriendsAsyncActionRequestUserInformation::HandleCallback()
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	AsyncTask(ENamedThreads::GameThread, [this]()
	{
		OnCallback.Broadcast();

		SetReadyToDestroy();
	});
#endif
}

#if WITH_STEAMCORE
void USteamCoreProFriendsAsyncActionRequestUserInformation::OnPersonaStateChange(PersonaStateChange_t* pParam)
{
	LogSteamCoreVerbose("");

	if (pParam->m_ulSteamID == m_SteamIDUser.GetValue())
	{
		OnPersonaStateChangeCallback.Unregister();
		HandleCallback();
	}
}
#endif
