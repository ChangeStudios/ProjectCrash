/**
* Copyright (C) 2017-2024 eelDev AB
*/

#include "SteamCoreProGameMode.h"
#include <GameFramework/GameSession.h>

void ASteamCoreProGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	ErrorMessage = GameSession->ApproveLogin(Options);
	FGameModeEvents::GameModePreLoginEvent.Broadcast(this, UniqueId, ErrorMessage);
}
