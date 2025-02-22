/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemSteamCorePrivate.h"
#include "OnlineSubsystemSteamCoreUtilities.generated.h"

#if WITH_STEAMCORE
namespace SteamCore
{
	FString SteamResultString(EResult Result);
	FString SteamChatMemberStateChangeString(EChatMemberStateChange StateChange);
	FString SteamChatRoomEnterResponseString(EChatRoomEnterResponse Response);
	FString SteamMatchMakingServerResponseString(EMatchMakingServerResponse Response);
	FString SteamP2PConnectError(EP2PSessionError InError);
	FString SteamVoiceResult(EVoiceResult Result);
	EOnlineServerConnectionStatus::Type SteamConnectionResult(const EResult Result);	
}
#endif


UCLASS(abstract)
class ONLINESUBSYSTEMSTEAMCORE_API USteamCoreSubsystemUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "SteamCore|Utilities")
	static bool SendSessionInviteToFriend(int32 LocalUserNum, const FString& FriendSteamId);
	
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "SteamCore|Utilities")
	static void GetSessionSettings(UObject* WorldContextObject, TArray<FSteamSessionSetting>& SessionSettings, FString& SessionName, int32& MaxPlayers, bool& bAllowInvites, bool& bUsesPresence, bool& bAllowJoinInProgress, bool& bAllowJoinViaPresence, bool& bAllowJoinViaPresenceFriendsOnly, bool& bAntiCheatProtected, bool& bUsesStats, bool& bShouldAdvertise, bool& bUseLobbiesIfAvailable, bool& bUseLobbiesVoiceChatIfAvailable);
	
	UFUNCTION(BlueprintCallable, Category = "SteamCore|Utilities")
	static void GetExtraSessionSettings(TArray<FSteamSessionSetting>& SessionSettings);
	
	UFUNCTION(BlueprintCallable, Category = "SteamCore|Utilities")
	static void FindFriendSessions();

	static TArray<FSteamSessionSetting> GetCurrentSessionSetings();
};