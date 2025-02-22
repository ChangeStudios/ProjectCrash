/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official Steamworks Documentation: https://partner.steamgames.com/doc/api/ISteamUserStats
*/

#pragma once

#include "SteamCorePro/SteamCoreProAsync.h"
#include "SteamUserStatsTypes.h"
#include "SteamUserStatsAsyncActions.generated.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		Delegate declarations
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFindLeaderboardAsyncDelegate, const FLeaderboardFindResult&, Data, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDownloadLeaderboardEntriesAsyncDelegate, const FLeaderboardScoresDownloaded&, Data, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRequestGlobalStatsAsyncDelegate, const FGlobalStatsReceived&, Data, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRequestGlobalAchievementPercentagesAsyncDelegate, const FGlobalAchievementPercentagesReady&, Data, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGetNumberOfCurrentPlayersAsyncDelegate, const FNumberOfCurrentPlayers&, Data, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUploadLeaderboardScoreAsyncDelegate, const FLeaderboardScoreUploaded&, Data, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFindOrCreateLeaderboardAsyncDelegate, const FFindOrCreateLeaderboardData&, Data, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRequestUserStatsAsyncDelegate, const FRequestUserStatsData&, Data, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRequestCurrentStatsAsyncDelegate, const FRequestCurrentStatsData&, Data, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDownloadLeaderboardEntriesForUsersAsyncDelegate, const FLeaderboardScoresDownloadedForUsers&, Data, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAttachLeaderboardUGCAsyncDelegate, const FAttachLeaderboardUGCData&, Data, bool, bWasSuccessful);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		USteamCoreProUserStatsAsyncActionFindLeaderboard
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
UCLASS()
class STEAMCOREPRO_API USteamCoreProUserStatsAsyncActionFindLeaderboard : public USteamCoreProAsyncAction
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FOnFindLeaderboardAsyncDelegate OnCallback;
public:
	/**
	* Gets a leaderboard by name.
	* You must call either this or FindOrCreateLeaderboard to obtain the leaderboard handle which is valid for the game session for
	* each leaderboard you wish to access prior to calling any other Leaderboard functions.
	*
	* @param	LeaderboardName		The name of the leaderboard to find. Must not be longer than k_cchLeaderboardNameMax.
	* @param	Timeout				How long we wait for this function to finish before aborting
	*/
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", DisplayName = "Find Leaderboard"), Category = "SteamCore|UserStats|Async")
	static USteamCoreProUserStatsAsyncActionFindLeaderboard* FindLeaderboardAsync(UObject* WorldContextObject, FString LeaderboardName, float Timeout = 10.f);
public:
	UFUNCTION()
	void HandleCallback(const FLeaderboardFindResult& Data, bool bWasSuccessful);
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		USteamCoreProUserStatsAsyncActionDownloadLeaderboardEntries
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
UCLASS()
class STEAMCOREPRO_API USteamCoreProUserStatsAsyncActionDownloadLeaderboardEntries : public USteamCoreProAsyncAction
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FOnDownloadLeaderboardEntriesAsyncDelegate OnCallback;
public:
	/**
	* Fetches a series of leaderboard entries for a specified leaderboard.
	* You can ask for more entries than exist, then this will return as many as do exist.
	* If you want to download entries for an arbitrary set of users, such as all of
	* the users on a server then you can use DownloadLeaderboardEntriesForUsers which takes an array of Steam IDs.
	*
	* @param	SteamLeaderboard	A leaderboard handle obtained from FindLeaderboard or FindOrCreateLeaderboard.
	* @param	Request				The type of Data request to make.
	* @param	RangeStart			The index to start downloading entries relative to eLeaderboardDataRequest.
	* @param	RangeEnd			The last index to retrieve entries for relative to eLeaderboardDataRequest.
	* @param	Timeout				How long we wait for this function to finish before aborting
	*/
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", DisplayName = "Download Leaderboard Entries"), Category = "SteamCore|UserStats|Async")
	static USteamCoreProUserStatsAsyncActionDownloadLeaderboardEntries* DownloadLeaderboardEntriesAsync(UObject* WorldContextObject, FSteamLeaderboard SteamLeaderboard, ESteamLeaderboardDataRequest Request, int32 RangeStart, int32 RangeEnd, float Timeout = 10.f);
public:
	UFUNCTION()
	void HandleCallback(const FLeaderboardScoresDownloaded& Data, bool bWasSuccessful);
};


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		USteamCoreProUserStatsAsyncActionDownloadRequestGlobalStats
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
UCLASS()
class STEAMCOREPRO_API USteamCoreProUserStatsAsyncActionDownloadRequestGlobalStats : public USteamCoreProAsyncAction
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FOnRequestGlobalStatsAsyncDelegate OnCallback;
public:
	/**
	* Asynchronously fetches global stats Data, which is available for stats marked as "aggregated" in the App Admin panel of the Steamworks website.
	* You must have called RequestCurrentStats and it needs to return successfully via its callback prior to calling this.
	*
	* @param	HistoryDays		How many days of day-by-day history to retrieve in addition to the overall totals. The limit is 60.
	* @param	Timeout		How long we wait for this function to finish before aborting
	*/
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", DisplayName = "Request Global Stats"), Category = "SteamCore|UserStats|Async")
	static USteamCoreProUserStatsAsyncActionDownloadRequestGlobalStats* RequestGlobalStatsAsync(UObject* WorldContextObject, int32 HistoryDays, float Timeout = 10.f);
public:
	UFUNCTION()
	void HandleCallback(const FGlobalStatsReceived& Data, bool bWasSuccessful);
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		USteamCoreProUserStatsAsyncActionRequestGlobalAchievementPercentages
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
UCLASS()
class STEAMCOREPRO_API USteamCoreProUserStatsAsyncActionRequestGlobalAchievementPercentages : public USteamCoreProAsyncAction
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FOnRequestGlobalAchievementPercentagesAsyncDelegate OnCallback;
public:
	/**
	* Asynchronously fetch the Data for the percentage of players who have received each achievement for the current game globally.
	* You must have called RequestCurrentStats and it needs to return successfully via its callback prior to calling this!
	* 
	* @param	Timeout		How long we wait for this function to finish before aborting
	*/
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", DisplayName = "Request Global Achievement Percentages"), Category = "SteamCore|UserStats|Async")
	static USteamCoreProUserStatsAsyncActionRequestGlobalAchievementPercentages* RequestGlobalAchievementPercentagesAsync(UObject* WorldContextObject, float Timeout = 10.f);

public:
	UFUNCTION()
	void HandleCallback(const FGlobalAchievementPercentagesReady& Data, bool bWasSuccessful);
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		USteamCoreProUserStatsAsyncActionGetNumberOfCurrentPlayers
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
UCLASS()
class STEAMCOREPRO_API USteamCoreProUserStatsAsyncActionGetNumberOfCurrentPlayers : public USteamCoreProAsyncAction
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FOnGetNumberOfCurrentPlayersAsyncDelegate OnCallback;
public:
	/**
	* Asynchronously retrieves the total number of players currently playing the current game. Both online and in offline mode.
	*
	* @param	Timeout		How long we wait for this function to finish before aborting
	*/
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", DisplayName = "Get Number Of Current Players"), Category = "SteamCore|UserStats|Async")
	static USteamCoreProUserStatsAsyncActionGetNumberOfCurrentPlayers* GetNumberOfCurrentPlayersAsync(UObject* WorldContextObject, float Timeout = 10.f);

public:
	UFUNCTION()
	void HandleCallback(const FNumberOfCurrentPlayers& Data, bool bWasSuccessful);
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		USteamCoreProUserStatsAsyncActionUploadLeaderboardScore
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
UCLASS()
class STEAMCOREPRO_API USteamCoreProUserStatsAsyncActionUploadLeaderboardScore : public USteamCoreProAsyncAction
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FOnUploadLeaderboardScoreAsyncDelegate OnCallback;
public:
	/**
	* Uploads a user score to a specified leaderboard.
	* Details are optional game-defined information which outlines how the user got that score. For example if it's a racing style time based leaderboard
	* you could store the timestamps when the player hits each checkpoint. If you have collectibles along the way you could use bit fields as booleans to store the items the player picked up in the playthrough.
	* Uploading scores to Steam is rate limited to 10 uploads per 10 minutes and you may only have one outstanding call to this function at a time.
	*
	* @param	SteamLeaderboard		A leaderboard handle obtained from FindLeaderboard or FindOrCreateLeaderboard.
	* @param	UploadScoreMethod		Do you want to force the score to change, or keep the previous score if it was better?
	* @param	Score					The score to upload.
	* @param	ScoreDetails			Optional: Array containing the details surrounding the unlocking of this score.
	* @param	Timeout		How long we wait for this function to finish before aborting
	*/
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", DisplayName = "Upload Leaderboard Score", AutoCreaterefterm="scoreDetails"), Category = "SteamCore|UserStats|Async")
	static USteamCoreProUserStatsAsyncActionUploadLeaderboardScore* UploadLeaderboardScoreAsync(UObject* WorldContextObject, FSteamLeaderboard SteamLeaderboard, ESteamLeaderboardUploadScoreMethod UploadScoreMethod, int32 Score, TArray<int32> ScoreDetails, float Timeout = 10.f);
public:
	UFUNCTION()
	void HandleCallback(const FLeaderboardScoreUploaded& Data, bool bWasSuccessful);
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		USteamCoreProUserStatsAsyncActionFindOrCreateLeaderboard
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
UCLASS()
class STEAMCOREPRO_API USteamCoreProUserStatsAsyncActionFindOrCreateLeaderboard : public USteamCoreProAsyncAction
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FOnFindOrCreateLeaderboardAsyncDelegate OnCallback;
public:
	/**
	* Gets a leaderboard by name, it will create it if it's not yet created.
	* You must call either this or FindLeaderboard to obtain the leaderboard handle which is valid for the
	* game session for each leaderboard you wish to access prior to calling any other Leaderboard functions.
	* Leaderboards created with this function will not automatically show up in the Steam Community.
	* You must manually set the Community Name field in the App Admin panel of the Steamworks website.
	* As such it's generally recommended to prefer creating the leaderboards in the App Admin panel on the Steamworks
	* website and using FindLeaderboard unless you're expected to have a large amount of dynamically created leaderboards.
	* You should never pass k_ELeaderboardSortMethodNone for eLeaderboardSortMethod or k_ELeaderboardDisplayTypeNone
	* for eLeaderboardDisplayType as this is undefined behavior.
	*
	* @param	LeaderboardName		The name of the leaderboard to find or create. Must not be longer than k_cchLeaderboardNameMax.
	* @param	SortMethod			The sort order of the new leaderboard if it's created.
	* @param	DisplayType			The display type (used by the Steam Community web site) of the new leaderboard if it's created.
	* @param	Timeout				How long we wait for this function to finish before aborting
	*/
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", DisplayName = "Find or Create Leaderboard"), Category = "SteamCore|UserStats|Async")
	static USteamCoreProUserStatsAsyncActionFindOrCreateLeaderboard* FindOrCreateLeaderboardAsync(UObject* WorldContextObject, FString LeaderboardName, ESteamLeaderboardSortMethod SortMethod, ESteamLeaderboardDisplayType DisplayType, float Timeout = 10.f);
public:
	UFUNCTION()
	void HandleCallback(const FFindOrCreateLeaderboardData& Data, bool bWasSuccessful);
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		USteamCoreProUserStatsAsyncActionRequestUserStats
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
UCLASS()
class STEAMCOREPRO_API USteamCoreProUserStatsAsyncActionRequestUserStats : public USteamCoreProAsyncAction
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FOnRequestUserStatsAsyncDelegate OnCallback;
public:
	/**
	* Asynchronously downloads stats and achievements for the specified user from the server.
	* These stats are not automatically updated; you'll need to call this function again to refresh any Data that may have change.
	* To keep from using too much memory, an least recently used cache (LRU) is maintained and other user's stats will occasionally be unloaded. When this happens a
	* UserStatsUnloaded_t callback is sent. After receiving this callback the user's stats will be unavailable until this function is called again.
	* The equivalent function for the local user is RequestCurrentStats, the equivalent function for game servers is ISteamGameServerStats::RequestUserStats.
	*
	* @param	SteamID		The Steam ID of the user to request stats for.
	* @param	Timeout		How long we wait for this function to finish before aborting
	*/
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", DisplayName = "Request User Stats"), Category = "SteamCore|UserStats|Async")
	static USteamCoreProUserStatsAsyncActionRequestUserStats* RequestUserStatsAsync(UObject* WorldContextObject, FSteamID SteamID, float Timeout = 10.f);
public:
	UFUNCTION()
	void HandleCallback(const FRequestUserStatsData& Data, bool bWasSuccessful);
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		USteamCoreProUserStatsAsyncActionRequestCurrentStats
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
UCLASS()
class STEAMCOREPRO_API USteamCoreProUserStatsAsyncActionRequestCurrentStats : public USteamCoreProAsyncAction
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FOnRequestCurrentStatsAsyncDelegate OnCallback;
public:
	/**
	* Note: this call is no longer required as it is managed by the Steam client
	* The game stats and achievements will be synchronized with Steam before
	* the game process begins.
	*/
	UFUNCTION(BlueprintCallable, meta = (Deprecated, WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", DisplayName = "Request Current Stats (Async)"), Category = "SteamCore|UserStats|Async")
	static USteamCoreProUserStatsAsyncActionRequestCurrentStats* RequestCurrentStatsAsync(UObject* WorldContextObject, float Timeout = 10.f);
public:
	virtual void Activate() override;
	
	UFUNCTION()
	void HandleCallback(const FRequestCurrentStatsData& Data, bool bWasSuccessful);
#if WITH_STEAMCORE
	STEAM_CALLBACK_MANUAL(USteamCoreProUserStatsAsyncActionRequestCurrentStats, OnUserStatsReceived, UserStatsReceived_t, OnUserStatsReceivedCallback);
#endif
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		USteamCoreProUserStatsAsyncActionDownloadLeaderboardEntriesForUsers
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
UCLASS()
class STEAMCOREPRO_API USteamCoreProUserStatsAsyncActionDownloadLeaderboardEntriesForUsers : public USteamCoreProAsyncAction
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FOnDownloadLeaderboardEntriesForUsersAsyncDelegate OnCallback;
public:
	/**
	* Fetches leaderboard entries for an arbitrary set of users on a specified leaderboard.
	* A maximum of 100 users can be downloaded at a time, with only one outstanding call at a time. If a user doesn't have an entry on the specified leaderboard, they won't be included in the result.
	* If you want to download entries based on their ranking or friends of the current user then you should use DownloadLeaderboardEntries.
	* You must call FindLeaderboard or FindOrCreateLeaderboard to get a SteamLeaderboard_t prior to calling this function.
	*
	* @param	SteamLeaderboard	A leaderboard handle obtained from FindLeaderboard or FindOrCreateLeaderboard.
	* @param	Users				An array of Steam IDs to get the leaderboard entries for.
	* @param	Timeout				How long we wait for this function to finish before aborting
	*/
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", DisplayName = "Download Leaderboard Entries for Users"), Category = "SteamCore|UserStats|Async")
	static USteamCoreProUserStatsAsyncActionDownloadLeaderboardEntriesForUsers* DownloadLeaderboardEntriesForUsersAsync(UObject* WorldContextObject, FSteamLeaderboard SteamLeaderboard, TArray<FSteamID> Users, float Timeout = 10.f);
public:
	UFUNCTION()
	void HandleCallback(const FLeaderboardScoresDownloadedForUsers& Data, bool bWasSuccessful);
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		USteamCoreProUserStatsAsyncActionAttachLeaderboardUGC
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
UCLASS()
class STEAMCOREPRO_API USteamCoreProUserStatsAsyncActionAttachLeaderboardUGC : public USteamCoreProAsyncAction
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FOnAttachLeaderboardUGCAsyncDelegate OnCallback;
public:
	/**
	* Attaches a piece of user generated content the current user's entry on a leaderboard.
	* This content could be a replay of the user achieving the score or a ghost to race against. The attached handle will be available when the entry is retrieved and can be accessed by other users using GetDownloadedLeaderboardEntry which contains LeaderboardEntry_t.m_hUGC. To create and download user generated content see the documentation for the Steam Workshop.
	* Once attached, the content will be available even if the underlying Cloud file is changed or deleted by the user.
	* You must call FindLeaderboard or FindOrCreateLeaderboard to get a SteamLeaderboard_t prior to calling this function.
	*
	* @param	SteamLeaderboard	A leaderboard handle obtained from FindLeaderboard or FindOrCreateLeaderboard.
	* @param	Handle				Handle to a piece of user generated content that was shared using ISteamRemoteStorage::FileShare or ISteamUGC::CreateItem.
	* @param	Timeout				How long we wait for this function to finish before aborting
	*/
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", DisplayName = "Attach Leaderboard UGC"), Category = "SteamCore|UserStats|Async")
	static USteamCoreProUserStatsAsyncActionAttachLeaderboardUGC* AttachLeaderboardUGCAsync(UObject* WorldContextObject, FSteamLeaderboard SteamLeaderboard, FSteamUGCHandle Handle, float Timeout = 10.f);
public:
	UFUNCTION()
	void HandleCallback(const FAttachLeaderboardUGCData& Data, bool bWasSuccessful);
};
