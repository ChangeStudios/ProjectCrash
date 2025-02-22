/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official Steamworks Documentation: https://partner.steamgames.com/doc/api/ISteamMatchmaking
*/

#pragma once

#include "SteamMatchmakingTypes.h"
#include "SteamCorePro/SteamCoreProAsync.h"
#include "Net/OnlineBlueprintCallProxyBase.h"
#if UE_VERSION_OLDER_THAN(5,2,0)
#include "Public/OnlineSessionSettings.h"
#else
#include "OnlineSessionSettings.h"
#endif
#include "Interfaces/OnlineSessionInterface.h"
#if UE_VERSION_NEWER_THAN(5,2,1)
#include <FindSessionsCallbackProxy.h>
#else
#include <OnlineSubsystemUtils/Classes/FindSessionsCallbackProxy.h>
#endif
#include "SteamMatchmakingAsyncActions.generated.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		Delegate declarations
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRequestLobbyListAsyncDelegate, const FLobbyMatchList&, Data, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCreateLobbyAsyncDelegate, const FCreateLobbyData&, Data, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnJoinLobbyAsyncDelegate, const FJoinLobbyData&, Data, bool, bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCreateSessionComplete, FName, bool);

using FOnCreateSessionCompleteDelegate = FOnCreateSessionComplete::FDelegate;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		USteamCoreProMatchmakingAsyncActionCreateLobby
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
UCLASS()
class STEAMCOREPRO_API USteamCoreProMatchmakingAsyncActionCreateLobby : public USteamCoreProAsyncAction
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FOnCreateLobbyAsyncDelegate OnCallback;
public:
	/**
	* Create a new matchmaking lobby.
	*
	* @param	LobbyType		The type and visibility of this lobby. This can be changed later via SetLobbyType.
	* @param	MaxMembers			The maximum number of players that can join this lobby. This can not be above 250.
	* @param	Timeout		How long we wait for this function to finish before aborting
	*/
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", DisplayName = "Create Lobby"), Category = "SteamCore|Matchmaking|Async")
	static USteamCoreProMatchmakingAsyncActionCreateLobby* CreateLobbyAsync(UObject* WorldContextObject, ESteamLobbyType LobbyType, int32 MaxMembers, float Timeout = 10.f);
public:
	UFUNCTION()
	void HandleCallback(const FCreateLobbyData& Data, bool bWasSuccessful);
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		USteamCoreProMatchmakingAsyncActionRequestLobbyList
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
UCLASS()
class STEAMCOREPRO_API USteamCoreProMatchmakingAsyncActionRequestLobbyList : public USteamCoreProAsyncAction
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FOnRequestLobbyListAsyncDelegate OnCallback;
public:
	/**
	* Get a filtered list of relevant lobbies.
	* There can only be one active lobby search at a time. The old request will be canceled if a new one is started. Depending on the users connection to the Steam back-end, this call can take from 300ms to 5 seconds to complete, and has a timeout of 20 seconds.
	* To filter the results you MUST call the AddRequestLobbyList* functions before calling this. The filters are cleared on each call to this function.
	* If AddRequestLobbyListDistanceFilter is not called, k_ELobbyDistanceFilterDefault will be used, which will only find matches in the same or nearby regions.
	* This will only return lobbies that are not full, and only lobbies that are k_ELobbyTypePublic or k_ELobbyTypeInvisible, and are set to joinable with SetLobbyJoinable.
	*
	* @param	Timeout		How long we wait for this function to finish before aborting
	* 
	*/
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", DisplayName = "Request Lobby List"), Category = "SteamCore|Matchmaking|Async")
	static USteamCoreProMatchmakingAsyncActionRequestLobbyList* RequestLobbyListAsync(UObject* WorldContextObject, float Timeout = 10.f);
public:
	UFUNCTION()
	void HandleCallback(const FLobbyMatchList& Data, bool bWasSuccessful);
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		USteamCoreProMatchmakingAsyncActionJoinLobby
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
UCLASS()
class STEAMCOREPRO_API USteamCoreProMatchmakingAsyncActionJoinLobby : public USteamCoreProAsyncAction
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FOnJoinLobbyAsyncDelegate OnCallback;
public:
	/**
	* Joins an existing lobby.
	* The lobby Steam ID can be obtained either from a search with RequestLobbyList, joining on a friend, or from an invite.
	*
	* @param	SteamIDLobby	The Steam ID of the lobby to join.
	* @param	Timeout			How long we wait for this function to finish before aborting
	*/
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", DisplayName = "Join Lobby"), Category = "SteamCore|Matchmaking|Async")
	static USteamCoreProMatchmakingAsyncActionJoinLobby* JoinLobbyAsync(UObject* WorldContextObject, FSteamID SteamIDLobby, float Timeout = 10.f);
public:
	UFUNCTION()
	void HandleCallback(const FJoinLobbyData& Data, bool bWasSuccessful);
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		USteamCoreProCreateSession
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
UCLASS()
class STEAMCOREPRO_API USteamCoreProCreateSession : public USteamCoreProAsyncAction
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FEmptyOnlineDelegate OnSuccess;
	UPROPERTY(BlueprintAssignable)
	FEmptyOnlineDelegate OnFailure;
public:
	USteamCoreProCreateSession();
	/**
	* Creates an online session
	* 
	* Automatically detects if we're running a dedicated or listen server
	*
	* Listen servers will create a Lobby Session, use "FindSteamCoreProSession" to find Listen Servers
	*
	*    @param SessionName							Name of our session
	*    @param MaxPlayers							Number of public connections
	*    @param bUsesPresence						Create a lobby for the session (disabled for dedicated servers)
	*    @param bUseLAN								Used for LAN server
	*    @param bUseLobbiesVoiceChatIfAvailable		(4.27+ only, Whether to create (and auto join) a voice chat room for the lobby, if the platform supports it)
	*    @param	Timeout								How long we wait for this function to finish before aborting
	*/
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", AutoCreateRefTerm = "sessionSettings"), Category = "SteamCore|Matchmaking|Async")
	static USteamCoreProCreateSession* CreateSteamCoreProSession(UObject* WorldContextObject, TMap<FString, FSteamSessionSetting> SessionSettings, FString SessionName = "SteamCoreProSession", int32 MaxPlayers = 5, bool bUseLAN = false, bool bAllowInvites = true, bool bUsesPresence = true, bool bAllowJoinViaPresence = true, bool bAllowJoinViaPresenceFriendsOnly = false, bool bAntiCheatProtected = false, bool bUsesStats = false, bool bShouldAdvertise = true, bool bUseLobbiesVoiceChatIfAvailable = true, float Timeout = 10.f);
protected:
	FOnlineSessionSettings m_SessionSettings;
	IOnlineSessionPtr m_SessionsPtr;
private:
	FOnCreateSessionCompleteDelegate Delegate;
	FDelegateHandle DelegateHandle;
public:
	virtual void Activate() override;
public:
	void OnCompleted(FName SessionName, bool bSuccessful);
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFindSteamSessionsResultDelegate, const TArray<FSteamSessionResult>&, Results);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		USteamCoreProFindSession
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
UCLASS()
class STEAMCOREPRO_API USteamCoreProFindSession : public USteamCoreProAsyncAction
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FFindSteamSessionsResultDelegate OnCallback;

public:
	USteamCoreProFindSession();
	/**
	* Find Online Sessions
	*
	* This is primarily used to find Listen Servers (lobbies), can be used to find dedicated servers but will give less server info
	*
	* @param	MaxResults				Max Number of Servers
	* @param	ServerType				Search for Listen or Dedicated servers
	* @param	DistanceFilter			Distance Filter for Listen servers (does not work for dedicated servers)
	* @param	bUseLAN					Search for LAN servers only
	* @param	bEmptyServersOnly		Only empty servers
	* @param	bSecureServersOnly		Only secure servers
	* @param	Timeout					How long we wait for this function to finish before aborting
	*/
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", AutoCreateRefTerm = "searchSettings"), Category = "SteamCore|Matchmaking|Async")
	static USteamCoreProFindSession* FindSteamCoreProSessions(UObject* WorldContextObject, TMap<FString, FSteamSessionSearchSetting> SearchSettings, int32 MaxResults = 50, bool bUseLAN = false, ESteamSessionFindType ServerType = ESteamSessionFindType::Listen, ESteamLobbyDistanceFilter DistanceFilter = ESteamLobbyDistanceFilter::k_ELobbyDistanceFilterWorldwide, bool bEmptyServersOnly = false, bool bSecureServersOnly = false, float Timeout = 10.f);

protected:
	TMap<FString, FSteamSessionSearchSetting> m_AdditionalSearchSettings;
	TSharedPtr<class FOnlineSessionSearchSteam> m_SearchSettings;
	TArray<FBlueprintSessionResult> m_SearchResults;
	int m_MaxResults;
	bool m_bUseLAN;
	ESteamSessionFindType m_ServerType;
	ESteamLobbyDistanceFilter m_DistanceFilter;
	bool m_bEmptyServersOnly;
	bool m_bSecureServersOnly;
private:
	FOnFindSessionsCompleteDelegate Delegate;
	FDelegateHandle DelegateHandle;
public:
	virtual void Activate() override;
public:
	UFUNCTION()
	void OnCompleted(bool bSuccessful);
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		USteamCoreProDestroySession
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
UCLASS()
class STEAMCOREPRO_API USteamCoreProDestroySession : public USteamCoreProAsyncAction
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FEmptyOnlineDelegate OnSuccess;
	UPROPERTY(BlueprintAssignable)
	FEmptyOnlineDelegate OnFailure;
public:
	USteamCoreProDestroySession();
	/**
	* Destroy an Online Session
	* @param	Timeout		How long we wait for this function to finish before aborting
	*/
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true"), Category = "SteamCore|Matchmaking|Async")
	static USteamCoreProDestroySession* DestroySteamCoreProSession(UObject* WorldContextObject, float Timeout = 10.f);
protected:
	FName SessionName;
private:
	FOnDestroySessionCompleteDelegate Delegate;
	FDelegateHandle DelegateHandle;
public:
	virtual void Activate() override;
public:
	UFUNCTION()
	void OnCompleted(FName sessionName, bool bWasSuccessful);
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		USteamCoreProUpdateSession
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
UCLASS()
class STEAMCOREPRO_API USteamCoreProUpdateSession : public USteamCoreProAsyncAction
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FEmptyOnlineDelegate OnCallback;
public:
	USteamCoreProUpdateSession();

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true"), Category = "SteamCore|Matchmaking|Async", meta = (AutoCreateRefTerm = "Settings"))
	static USteamCoreProUpdateSession* UpdateSteamCoreProSession(UObject* WorldContextObject, TMap<FString, FSteamSessionSetting> Settings, FString SessionName = "SteamCoreProSession", int32 MaxPlayers = 4, bool bAllowJoinInProgress = true);
protected:
	UObject* m_WorldContextObject;
private:
	FOnUpdateSessionCompleteDelegate Delegate;
	FDelegateHandle DelegateHandle;
public:
	virtual void Activate() override;
public:
	UFUNCTION()
	void OnCompleted(FName SessionName, bool bWasSuccessful);
};
