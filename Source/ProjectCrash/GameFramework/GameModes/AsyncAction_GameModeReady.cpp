// Copyright Samuel Reitich. All rights reserved.

#include "GameFramework/GameModes/AsyncAction_GameModeReady.h"

#include "GameModeManagerComponent.h"

UAsyncAction_GameModeReady* UAsyncAction_GameModeReady::WaitForGameModeReady(UObject* WorldContextObject)
{
	UAsyncAction_GameModeReady* Action = nullptr;

	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		Action = NewObject<UAsyncAction_GameModeReady>();
		Action->WorldPtr = World;
		Action->RegisterWithGameInstance(World);
	}

	return Action;
}

void UAsyncAction_GameModeReady::Activate()
{
	if (UWorld* World = WorldPtr.Get())
	{
		// If our game state is ready, start listening for it to load the game mode data.
		if (AGameStateBase* GameState = World->GetGameState())
		{
			Step2_ListenForGameModeDataLoaded(GameState);
		}
		// If our game state hasn't been initialized yet, wait for it before we start listening for the game mode data.
		else
		{
			World->GameStateSetEvent.AddUObject(this, &UAsyncAction_GameModeReady::Step1_HandleGameStateSet);
		}
	}
	// If we don't have a world, we can't listen for the game mode to be ready.
	else
	{
		UE_LOG(LogBlueprint, Error, TEXT("Node WaitForGameModeReady in [%s] called without a valid world. Callback event will never be triggered!"), *GetNameSafe(GetOuter()));
		SetReadyToDestroy();
	}
}

void UAsyncAction_GameModeReady::Step1_HandleGameStateSet(AGameStateBase* GameState)
{
	if (UWorld* World = WorldPtr.Get())
	{
		World->GameStateSetEvent.RemoveAll(this);
	}

	Step2_ListenForGameModeDataLoaded(GameState);
}

void UAsyncAction_GameModeReady::Step2_ListenForGameModeDataLoaded(AGameStateBase* GameState)
{
	check(GameState);
	UGameModeManagerComponent* GameModeManager = GameState->FindComponentByClass<UGameModeManagerComponent>();
	check(GameModeManager);

	// If the game mode is already loaded, fire the callback immediately.
	if (GameModeManager->IsGameModeLoaded())
	{
		UWorld* World = GameState->GetWorld();
		check(World);

		/* Delay a tick for any scripts that call this node before other initialization logic, but need the callback
		 * event to fire afterwards. */
		World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &UAsyncAction_GameModeReady::Step4_BroadcastReady));
	}
	// If the game mode is not loaded yet, listen for it to finish loading.
	else
	{
		GameModeManager->CallOrRegister_OnGameModeLoaded(FCrashGameModeLoadedSignature::FDelegate::CreateUObject(this, &UAsyncAction_GameModeReady::Step3_HandleGameModeDataLoaded));
	}
}

void UAsyncAction_GameModeReady::Step3_HandleGameModeDataLoaded(const UCrashGameModeData* CurrentGameModeData)
{
	Step4_BroadcastReady();
}

void UAsyncAction_GameModeReady::Step4_BroadcastReady()
{
	// Trigger the callback event.
	OnReady.Broadcast();

	// Game mode data will never be loaded twice.
	SetReadyToDestroy();
}
