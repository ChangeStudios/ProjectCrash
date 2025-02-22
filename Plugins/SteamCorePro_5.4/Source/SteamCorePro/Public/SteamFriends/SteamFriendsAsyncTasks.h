/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official Steamworks Documentation: https://partner.steamgames.com/doc/api/ISteamFriends
*/

#pragma once

#include "SteamFriendsTypes.h"
#include "SteamCorePro/SteamCoreProAsync.h"

#if WITH_STEAMCORE
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProFriendsSetPersonaName
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProFriendsSetPersonaName : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnSetPersonaName m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProFriendsSetPersonaName(const FOnSetPersonaName Callback, const FString Name, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , m_Name(Name)
	{
	}

	FOnlineAsyncTaskSteamCoreProFriendsSetPersonaName(USteamCoreProAsyncAction* AsyncObject, const FString Name, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_CallbackResults()
		  , m_Name(Name)
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
	}

private:
	FOnlineAsyncTaskSteamCoreProFriendsSetPersonaName() = delete;
protected:
	SetPersonaNameResponse_t m_CallbackResults;
	FString m_Name;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProFriendsSetPersonaName")); }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProFriendsDownloadClanActivityCounts
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProFriendsDownloadClanActivityCounts : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnDownloadClanActivityCounts m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProFriendsDownloadClanActivityCounts(const FOnDownloadClanActivityCounts Callback, const TArray<FSteamID> SteamIDClans, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , m_SteamIDClans(SteamIDClans)
	{
	}

	FOnlineAsyncTaskSteamCoreProFriendsDownloadClanActivityCounts(USteamCoreProAsyncAction* AsyncObject, const TArray<FSteamID> SteamIDClans, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_CallbackResults()
		  , m_SteamIDClans(SteamIDClans)
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
	}

private:
	FOnlineAsyncTaskSteamCoreProFriendsDownloadClanActivityCounts() = delete;
protected:
	DownloadClanActivityCountsResult_t m_CallbackResults;
	TArray<FSteamID> m_SteamIDClans;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProFriendsDownloadClanActivityCounts")); }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProRequestClanOfficerList
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProRequestClanOfficerList : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnRequestClanOfficerList m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProRequestClanOfficerList(const FOnRequestClanOfficerList Callback, const FSteamID SteamId, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , m_SteamID(SteamId)
	{
	}

	FOnlineAsyncTaskSteamCoreProRequestClanOfficerList(USteamCoreProAsyncAction* AsyncObject, const FSteamID SteamId, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_CallbackResults()
		  , m_SteamID(SteamId)
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
	}

private:
	FOnlineAsyncTaskSteamCoreProRequestClanOfficerList() = delete;
protected:
	ClanOfficerListResponse_t m_CallbackResults;
	FSteamID m_SteamID;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProRequestClanOfficerList")); }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProFriendsJoinClanChatRoom
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProFriendsJoinClanChatRoom : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnJoinClanChatRoom m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProFriendsJoinClanChatRoom(FOnJoinClanChatRoom Callback, const FSteamID SteamIdClan, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , m_SteamIDClan(SteamIdClan)
	{
	}

	FOnlineAsyncTaskSteamCoreProFriendsJoinClanChatRoom(USteamCoreProAsyncAction* AsyncObject, const FSteamID SteamIdClan, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_CallbackResults()
		  , m_SteamIDClan(SteamIdClan)
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
	}

private:
	FOnlineAsyncTaskSteamCoreProFriendsJoinClanChatRoom() = delete;
protected:
	JoinClanChatRoomCompletionResult_t m_CallbackResults;
	FSteamID m_SteamIDClan;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProFriendsJoinClanChatRoom")); }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProFriendsEnumerateFollowingList
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProFriendsEnumerateFollowingList : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnEnumerateFollowingList m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProFriendsEnumerateFollowingList(const FOnEnumerateFollowingList Callback, const int32 StartIndex, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , m_StartIndex(StartIndex)
	{
	}

	FOnlineAsyncTaskSteamCoreProFriendsEnumerateFollowingList(USteamCoreProAsyncAction* AsyncObject, const int32 StartIndex, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_CallbackResults()
		  , m_StartIndex(StartIndex)
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
	}

private:
	FOnlineAsyncTaskSteamCoreProFriendsEnumerateFollowingList() = delete;
protected:
	FriendsEnumerateFollowingList_t m_CallbackResults;
	int32 m_StartIndex;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProFriendsEnumerateFollowingList")); }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProFriendsIsFollowing
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProFriendsIsFollowing : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnIsFollowing m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProFriendsIsFollowing(FOnIsFollowing Callback, const FSteamID SteamId, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , m_SteamID(SteamId)
	{
	}

	FOnlineAsyncTaskSteamCoreProFriendsIsFollowing(USteamCoreProAsyncAction* AsyncObject, const FSteamID SteamId, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_CallbackResults()
		  , m_SteamID(SteamId)
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
	}

private:
	FOnlineAsyncTaskSteamCoreProFriendsIsFollowing() = delete;
protected:
	FriendsIsFollowing_t m_CallbackResults;
	FSteamID m_SteamID;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProFriendsIsFollowing")); }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProFriendsGetFollowerCount
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProFriendsGetFollowerCount : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnGetFollowerCount m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProFriendsGetFollowerCount(FOnGetFollowerCount Callback, const FSteamID SteamId, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , m_SteamID(SteamId)
	{
	}

	FOnlineAsyncTaskSteamCoreProFriendsGetFollowerCount(USteamCoreProAsyncAction* AsyncObject, const FSteamID SteamId, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_CallbackResults()
		  , m_SteamID(SteamId)
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
	}

private:
	FOnlineAsyncTaskSteamCoreProFriendsGetFollowerCount() = delete;
protected:
	FriendsGetFollowerCount_t m_CallbackResults;
	FSteamID m_SteamID;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProFriendsGetFollowerCount")); }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProRequestEquippedProfileItems
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProRequestEquippedProfileItems : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnRequestEquippedProfileItems m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProRequestEquippedProfileItems(const FOnRequestEquippedProfileItems Callback, const FSteamID SteamId, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , m_SteamId(SteamId)
	{
	}

	FOnlineAsyncTaskSteamCoreProRequestEquippedProfileItems(USteamCoreProAsyncAction* AsyncObject, const FSteamID SteamId, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_CallbackResults()
		  , m_SteamId(SteamId)
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
	}

private:
	FOnlineAsyncTaskSteamCoreProRequestEquippedProfileItems() = delete;
protected:
	EquippedProfileItems_t m_CallbackResults;
	FSteamID m_SteamId;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProRequestEquippedProfileItems")); }
};
#endif