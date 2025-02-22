/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official Steamworks Documentation: https://partner.steamgames.com/doc/api/ISteamUGC
*/

#pragma once

#include "SteamCorePro/SteamCoreProAsync.h"
#include "SteamUGCTypes.h"

#if WITH_STEAMCORE
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProUGCPlaytimeTrackingForAllItems
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProUGCStopPlaytimeTrackingForAllItems : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnStopPlaytimeTrackingForAllItems m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProUGCStopPlaytimeTrackingForAllItems(const FOnStopPlaytimeTrackingForAllItems Callback, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
	{
	}

	FOnlineAsyncTaskSteamCoreProUGCStopPlaytimeTrackingForAllItems(USteamCoreProAsyncAction* AsyncObject, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_CallbackResults()
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
	}

private:
	FOnlineAsyncTaskSteamCoreProUGCStopPlaytimeTrackingForAllItems() = delete;
protected:
	StopPlaytimeTrackingResult_t m_CallbackResults;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProUGCStopPlaytimeTrackingForAllItems")); }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProUGCStopPlaytimeTracking
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProUGCStopPlaytimeTracking : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnStopPlaytimeTracking m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProUGCStopPlaytimeTracking(const FOnStopPlaytimeTracking Callback, const TArray<FPublishedFileID> FileIDs, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , m_FileIDs(FileIDs)
	{
	}

	FOnlineAsyncTaskSteamCoreProUGCStopPlaytimeTracking(USteamCoreProAsyncAction* AsyncObject, const TArray<FPublishedFileID> FileIDs, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_CallbackResults()
		  , m_FileIDs(FileIDs)
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
	}

private:
	FOnlineAsyncTaskSteamCoreProUGCStopPlaytimeTracking() = delete;
protected:
	StopPlaytimeTrackingResult_t m_CallbackResults;
	TArray<FPublishedFileID> m_FileIDs;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProUGCStopPlaytimeTracking")); }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProUGCStartPlaytimeTracking
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProUGCStartPlaytimeTracking : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnStartPlaytimeTracking m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProUGCStartPlaytimeTracking(const FOnStartPlaytimeTracking Callback, const TArray<FPublishedFileID> FileIDs, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , m_FileIDs(FileIDs)
	{
	}

	FOnlineAsyncTaskSteamCoreProUGCStartPlaytimeTracking(USteamCoreProAsyncAction* AsyncObject, const TArray<FPublishedFileID> FileIDs, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_CallbackResults()
		  , m_FileIDs(FileIDs)
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
	}

private:
	FOnlineAsyncTaskSteamCoreProUGCStartPlaytimeTracking() = delete;
protected:
	StartPlaytimeTrackingResult_t m_CallbackResults;
	TArray<FPublishedFileID> m_FileIDs;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProUGCStartPlaytimeTracking")); }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProUGCUnsubscribeItem
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProUGCUnsubscribeItem : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnUnsubscribeItem m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProUGCUnsubscribeItem(const FOnUnsubscribeItem Callback, const FPublishedFileID FileID, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , m_FileID(FileID)
	{
	}

	FOnlineAsyncTaskSteamCoreProUGCUnsubscribeItem(USteamCoreProAsyncAction* AsyncObject, const FPublishedFileID PublishedFileID, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_CallbackResults()
		  , m_FileID(PublishedFileID)
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
	}

private:
	FOnlineAsyncTaskSteamCoreProUGCUnsubscribeItem() = delete;
protected:
	RemoteStorageSubscribePublishedFileResult_t m_CallbackResults;
	FPublishedFileID m_FileID;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProUGCUnsubscribeItem")); }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProUGCSubscribeItem
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProUGCSubscribeItem : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnSubscribeItem m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProUGCSubscribeItem(const FOnSubscribeItem Callback, const FPublishedFileID FileID, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , m_FileID(FileID)
	{
	}

	FOnlineAsyncTaskSteamCoreProUGCSubscribeItem(USteamCoreProAsyncAction* AsyncObject, const FPublishedFileID PublishedFileID, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_CallbackResults()
		  , m_FileID(PublishedFileID)
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
	}

private:
	FOnlineAsyncTaskSteamCoreProUGCSubscribeItem() = delete;
protected:
	RemoteStorageSubscribePublishedFileResult_t m_CallbackResults;
	FPublishedFileID m_FileID;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProUGCSubscribeItem")); }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProUGCRemoveItemFromFavorites
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProUGCRemoveItemFromFavorites : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnRemoveItemFromFavorites m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProUGCRemoveItemFromFavorites(const FOnRemoveItemFromFavorites Callback, const int32 AppID, const FPublishedFileID FileID, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , m_AppID(AppID)
		  , m_FileID(FileID)
	{
	}

	FOnlineAsyncTaskSteamCoreProUGCRemoveItemFromFavorites(USteamCoreProAsyncAction* AsyncObject, const int32 AppID, const FPublishedFileID FileID, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_CallbackResults()
		  , m_AppID(AppID)
		  , m_FileID(FileID)
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
	}

private:
	FOnlineAsyncTaskSteamCoreProUGCRemoveItemFromFavorites() = delete;
protected:
	UserFavoriteItemsListChanged_t m_CallbackResults;
	int32 m_AppID;
	FPublishedFileID m_FileID;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProUGCRemoveItemFromFavorites")); }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProUGCAddItemToFavorites
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProUGCAddItemToFavorites : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnAddItemToFavorites m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProUGCAddItemToFavorites(const FOnAddItemToFavorites Callback, const int32 AppID, const FPublishedFileID FileID, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , m_AppID(AppID)
		  , m_FileID(FileID)
	{
	}

	FOnlineAsyncTaskSteamCoreProUGCAddItemToFavorites(USteamCoreProAsyncAction* AsyncObject, const int32 AppID, const FPublishedFileID FileID, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_CallbackResults()
		  , m_AppID(AppID)
		  , m_FileID(FileID)
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
	}

private:
	FOnlineAsyncTaskSteamCoreProUGCAddItemToFavorites() = delete;
protected:
	UserFavoriteItemsListChanged_t m_CallbackResults;
	int32 m_AppID;
	FPublishedFileID m_FileID;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProUGCAddItemToFavorites")); }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProUGCGetUserItemVote
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProUGCGetUserItemVote : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnGetUserItemVote m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProUGCGetUserItemVote(const FOnGetUserItemVote Callback, const FPublishedFileID FileID, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , m_FileID(FileID)
	{
	}

	FOnlineAsyncTaskSteamCoreProUGCGetUserItemVote(USteamCoreProAsyncAction* AsyncObject, const FPublishedFileID FileID, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_CallbackResults()
		  , m_FileID(FileID)
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
	}

private:
	FOnlineAsyncTaskSteamCoreProUGCGetUserItemVote() = delete;
protected:
	GetUserItemVoteResult_t m_CallbackResults;
	FPublishedFileID m_FileID;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProUGCGetUserItemVote")); }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProUGCSetUserItemVote
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProUGCSetUserItemVote : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnSetUserItemVote m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProUGCSetUserItemVote(FOnSetUserItemVote Callback, const bool bVoteUp, const FPublishedFileID FileID, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , bVoteUp(bVoteUp)
		  , m_FileID(FileID)
	{
	}

	FOnlineAsyncTaskSteamCoreProUGCSetUserItemVote(USteamCoreProAsyncAction* AsyncObject, const bool bVoteUp, const FPublishedFileID FileID, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_CallbackResults()
		  , bVoteUp(bVoteUp)
		  , m_FileID(FileID)
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
	}

private:
	FOnlineAsyncTaskSteamCoreProUGCSetUserItemVote() = delete;
protected:
	SetUserItemVoteResult_t m_CallbackResults;
	bool bVoteUp;
	FPublishedFileID m_FileID;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProUGCSetUserItemVote")); }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProUGCSubmitItemUpdate
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProUGCSubmitItemUpdate : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnSubmitItemUpdate m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProUGCSubmitItemUpdate(const FOnSubmitItemUpdate Callback, const FUGCUpdateHandle Handle, const FString ChangeNote, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , m_UGCUpdateHandle(Handle)
		  , m_ChangeNote(ChangeNote)
	{
	}

	FOnlineAsyncTaskSteamCoreProUGCSubmitItemUpdate(USteamCoreProAsyncAction* AsyncObject, const FUGCUpdateHandle Handle, const FString ChangeNote, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_CallbackResults()
		  , m_UGCUpdateHandle(Handle)
		  , m_ChangeNote(ChangeNote)
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
	}

private:
	FOnlineAsyncTaskSteamCoreProUGCSubmitItemUpdate() = delete;
protected:
	SubmitItemUpdateResult_t m_CallbackResults;
	FUGCUpdateHandle m_UGCUpdateHandle;
	FString m_ChangeNote;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProUGCSubmitItemUpdate")); }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProUGCCreateItem
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProUGCCreateItem : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnCreateItem m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProUGCCreateItem(FOnCreateItem Callback, const int32 AppID, const ESteamWorkshopFileType FileType, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , m_ConsumerAppID(AppID)
		  , m_FileType(FileType)
	{
	}

	FOnlineAsyncTaskSteamCoreProUGCCreateItem(USteamCoreProAsyncAction* AsyncObject, const int32 AppID, const ESteamWorkshopFileType FileType, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_CallbackResults()
		  , m_ConsumerAppID(AppID)
		  , m_FileType(FileType)
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
	}

private:
	FOnlineAsyncTaskSteamCoreProUGCCreateItem() = delete;
protected:
	CreateItemResult_t m_CallbackResults;
	int32 m_ConsumerAppID;
	ESteamWorkshopFileType m_FileType;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProUGCCreateItem")); }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProUGCSendQueryUGCRequest
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProUGCSendQueryUGCRequest : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnSendQueryUGCRequest m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProUGCSendQueryUGCRequest(const FOnSendQueryUGCRequest Callback, const FUGCQueryHandle Handle, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , m_UGCQueryHandle(Handle)
	{
	}

	FOnlineAsyncTaskSteamCoreProUGCSendQueryUGCRequest(USteamCoreProAsyncAction* AsyncObject, const FUGCQueryHandle Handle, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_CallbackResults()
		  , m_UGCQueryHandle(Handle)
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
	}

private:
	FOnlineAsyncTaskSteamCoreProUGCSendQueryUGCRequest() = delete;
protected:
	SteamUGCQueryCompleted_t m_CallbackResults;
	FUGCQueryHandle m_UGCQueryHandle;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProUGCSendQueryUGCRequest")); }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProUGCAddAppDependency
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProUGCAddAppDependency : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnAddAppDependencyResult m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProUGCAddAppDependency(const FOnAddAppDependencyResult Callback, const FPublishedFileID PublishedFileID, const int32 AppID, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , m_PublishedFileID(PublishedFileID)
		  , m_AppID(AppID)
	{
	}

	FOnlineAsyncTaskSteamCoreProUGCAddAppDependency(USteamCoreProAsyncAction* AsyncObject, const FPublishedFileID PublishedFileID, const int32 AppID, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_CallbackResults()
		  , m_PublishedFileID(PublishedFileID)
		  , m_AppID(AppID)
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
	}

private:
	FOnlineAsyncTaskSteamCoreProUGCAddAppDependency() = delete;
protected:
	AddAppDependencyResult_t m_CallbackResults;
	FPublishedFileID m_PublishedFileID;
	int32 m_AppID;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProUGCAddAppDependency")); }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProUGCRemoveAppDependency
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProUGCRemoveAppDependency : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnRemoveAppDependencyResult m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProUGCRemoveAppDependency(const FOnRemoveAppDependencyResult Callback, const FPublishedFileID PublishedFileID, const int32 AppID, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , m_PublishedFileID(PublishedFileID)
		  , m_AppID(AppID)
	{
	}

	FOnlineAsyncTaskSteamCoreProUGCRemoveAppDependency(USteamCoreProAsyncAction* AsyncObject, const FPublishedFileID PublishedFileID, const int32 AppID, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_CallbackResults()
		  , m_PublishedFileID(PublishedFileID)
		  , m_AppID(AppID)
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
	}

private:
	FOnlineAsyncTaskSteamCoreProUGCRemoveAppDependency() = delete;
protected:
	RemoveAppDependencyResult_t m_CallbackResults;
	FPublishedFileID m_PublishedFileID;
	int32 m_AppID;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProUGCRemoveAppDependency")); }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProUGCAddUGCDependency
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProUGCAddUGCDependency : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnAddUGCDependencyResult m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProUGCAddUGCDependency(const FOnAddUGCDependencyResult Callback, const FPublishedFileID PublishedFileId, const FPublishedFileID ChildPublishedFileID, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , m_PublishedFileID(PublishedFileId)
		  , m_ChildPublishedFileID(ChildPublishedFileID)
	{
	}

	FOnlineAsyncTaskSteamCoreProUGCAddUGCDependency(USteamCoreProAsyncAction* AsyncObject, const FPublishedFileID PublishedFileId, const FPublishedFileID ChildPublishedFileID, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_CallbackResults()
		  , m_PublishedFileID(PublishedFileId)
		  , m_ChildPublishedFileID(ChildPublishedFileID)
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
	}

private:
	FOnlineAsyncTaskSteamCoreProUGCAddUGCDependency() = delete;
protected:
	AddUGCDependencyResult_t m_CallbackResults;
	FPublishedFileID m_PublishedFileID;
	FPublishedFileID m_ChildPublishedFileID;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProUGCAddUGCDependency")); }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProUGCRemoveUGCDependency
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProUGCRemoveUGCDependency : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnRemoveUGCDependencyResult m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProUGCRemoveUGCDependency(const FOnRemoveUGCDependencyResult Callback, const FPublishedFileID ParentPublishedFileId, const FPublishedFileID ChildPublishedFileID, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , m_ParentPublishedFileID(ParentPublishedFileId)
		  , m_ChildPublishedFileID(ChildPublishedFileID)
	{
	}

	FOnlineAsyncTaskSteamCoreProUGCRemoveUGCDependency(USteamCoreProAsyncAction* AsyncObject, const FPublishedFileID ParentPublishedFileId, const FPublishedFileID ChildPublishedFileID, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_CallbackResults()
		  , m_ParentPublishedFileID(ParentPublishedFileId)
		  , m_ChildPublishedFileID(ChildPublishedFileID)
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
	}

private:
	FOnlineAsyncTaskSteamCoreProUGCRemoveUGCDependency() = delete;
protected:
	RemoveUGCDependencyResult_t m_CallbackResults;
	FPublishedFileID m_ParentPublishedFileID;
	FPublishedFileID m_ChildPublishedFileID;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProUGCRemoveUGCDependency")); }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProUGCDeleteItem
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProUGCDeleteItem : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnDeleteItemResult m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProUGCDeleteItem(const FOnDeleteItemResult Callback, const FPublishedFileID PublishedFileID, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , m_PublishedFileID(PublishedFileID)
	{
	}

	FOnlineAsyncTaskSteamCoreProUGCDeleteItem(USteamCoreProAsyncAction* AsyncObject, const FPublishedFileID PublishedFileID, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_CallbackResults()
		  , m_PublishedFileID(PublishedFileID)
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
	}

private:
	FOnlineAsyncTaskSteamCoreProUGCDeleteItem() = delete;
protected:
	DeleteItemResult_t m_CallbackResults;
	FPublishedFileID m_PublishedFileID;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProUGCDeleteItem")); }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProUGCDeleteItem
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProUGCGetWorkshopEULAStatus : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnGetWorkshopEULAStatus m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProUGCGetWorkshopEULAStatus(const FOnGetWorkshopEULAStatus Callback, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
	{
	}

	FOnlineAsyncTaskSteamCoreProUGCGetWorkshopEULAStatus(USteamCoreProAsyncAction* AsyncObject, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_CallbackResults()
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
	}

private:
	FOnlineAsyncTaskSteamCoreProUGCGetWorkshopEULAStatus() = delete;
protected:
	WorkshopEULAStatus_t m_CallbackResults;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProUGCGetWorkshopEULAStatus")); }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProUGCGetAppDependencies
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProUGCGetAppDependencies : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnGetAppDependenciesResult m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProUGCGetAppDependencies(const FOnGetAppDependenciesResult Callback, const FPublishedFileID PublishedFileID, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , m_PublishedFileID(PublishedFileID)
	{
	}

	FOnlineAsyncTaskSteamCoreProUGCGetAppDependencies(USteamCoreProAsyncAction* AsyncObject, const FPublishedFileID PublishedFileID, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_CallbackResults()
		  , m_PublishedFileID(PublishedFileID)
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
	}

private:
	FOnlineAsyncTaskSteamCoreProUGCGetAppDependencies() = delete;
protected:
	GetAppDependenciesResult_t m_CallbackResults;
	FPublishedFileID m_PublishedFileID;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProUGCGetAppDependencies")); }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProUGCDownloadItem
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProUGCDownloadItem : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnDownloadItem m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProUGCDownloadItem(const FOnDownloadItem Callback, const FPublishedFileID PublishedFileID, const bool bHighPriority, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_PublishedFileID(PublishedFileID)
		  , bHighPriority(bHighPriority)
	{
		OnDownloadItemResultCallback.Register(this, &FOnlineAsyncTaskSteamCoreProUGCDownloadItem::OnDownloadItemResult);
	}

	FOnlineAsyncTaskSteamCoreProUGCDownloadItem(USteamCoreProAsyncAction* AsyncObject, const FPublishedFileID PublishedFileID, const bool bHighPriority, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_PublishedFileID(PublishedFileID)
		  , bHighPriority(bHighPriority)
	{
		OnDownloadItemResultCallback.Register(this, &FOnlineAsyncTaskSteamCoreProUGCDownloadItem::OnDownloadItemResult);
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
	}

	virtual ~FOnlineAsyncTaskSteamCoreProUGCDownloadItem() override
	{
		OnDownloadItemResultCallback.Unregister();
	}

private:
	FOnlineAsyncTaskSteamCoreProUGCDownloadItem() = delete;
protected:
	DownloadItemResult_t m_CallbackResults;
	FPublishedFileID m_PublishedFileID;
	bool bHighPriority;
protected:
#if WITH_STEAMCORE
	STEAM_CALLBACK_MANUAL(FOnlineAsyncTaskSteamCoreProUGCDownloadItem, OnDownloadItemResult, DownloadItemResult_t, OnDownloadItemResultCallback);
#endif
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProUGCDownloadItem")); }
};
#endif