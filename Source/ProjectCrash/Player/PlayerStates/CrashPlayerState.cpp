// Copyright Samuel Reitich. All rights reserved.


#include "Player/PlayerStates/CrashPlayerState.h"

#include "CrashGameplayTags.h"
#include "AbilitySystem/Abilities/CrashAbilitySet.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "Characters/Data/PawnData.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GameFramework/CrashAssetManager.h"
#include "GameFramework/CrashLogging.h"
#include "GameFramework/GameModes/CrashGameMode.h"
#include "GameFramework/GameModes/GameModeManagerComponent.h"
#include "Net/UnrealNetwork.h"

const FName ACrashPlayerState::NAME_AbilitiesReady("AbilitiesReady");

void ACrashPlayerState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Listen for the game mode to finish loading so we can use it to initialize our pawn data.
	UWorld* World = GetWorld();
	if (World && World->IsGameWorld() && World->GetNetMode() != NM_Client)
	{
		AGameStateBase* GS = GetWorld()->GetGameState();
		check(GS);
		UGameModeManagerComponent* GameModeManagerComponent = GS->FindComponentByClass<UGameModeManagerComponent>();
		check(GameModeManagerComponent);
		GameModeManagerComponent->CallOrRegister_OnGameModeLoaded(
			FCrashGameModeLoadedSignature::FDelegate::CreateUObject(this, &ThisClass::OnGameModeLoaded),
			ECrashGameModeLoadedResponsePriority::First);
	}
}

void ACrashPlayerState::OnGameModeLoaded(const UCrashGameModeData* GameModeData)
{
	// Use the game mode to initialize the pawn data.
	if (ACrashGameMode* CrashGM = GetWorld()->GetAuthGameMode<ACrashGameMode>())
	{
		if (const UPawnData* NewPawnData = CrashGM->FindPawnDataForController(GetOwningController()))
		{
			SetPawnData(NewPawnData);
		}
		else
		{
			UE_LOG(LogCrashGameMode, Error, TEXT("ACrashPlayerState::OnGameModeLoaded unable to find pawn data to initialize player state [%s]."), *GetNameSafe(this));
		}
	}
}

void ACrashPlayerState::SetPawnData(const UPawnData* InPawnData)
{
	// Pawn data can only be changed by the server.
	if (!HasAuthority())
	{
		return;
	}

	// Players cannot change pawn data during the game.
	if (PawnData)
	{
		UE_LOG(LogCrash, Error, TEXT("Tried to set pawn data [%s] on player state [%s]. This player already has valid pawn data: [%s]."),
			*GetNameSafe(InPawnData),
			*GetNameSafe(this),
			*GetNameSafe(PawnData));

		/* Note: If we wanted a game mode where players can switch pawns during a game (e.g. Overwatch), we would have
		 * to remove the current pawn data's added ability sets and input configuration. */
		return;
	}

	UE_LOG(LogCrashGameMode, Log, TEXT("Set pawn data [%s] for player [%s]. (%s)"),
		*GetNameSafe(InPawnData),
		*GetNameSafe(this),
		*GetClientServerContextString(this));

	// Cache and replicate the new pawn data.
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, PawnData, this);
	PawnData = InPawnData;

	// Grant the pawn's default ability sets.
	for (const UCrashAbilitySet* AbilitySet : PawnData->AbilitySets)
	{
		if (AbilitySet)
		{
			// AbilitySet->GiveToAbilitySystem(UCrashAbilitySystemComponent, nullptr);
		}
	}

	/* Fire a game framework component extension event notifying the manager that this player received its pawn's
	 * default ability sets. */
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, NAME_AbilitiesReady);

	ForceNetUpdate();
}

void ACrashPlayerState::OnRep_PawnData()
{
	UE_LOG(LogCrashGameMode, Log, TEXT("Set pawn data [%s] for player [%s]. (%s)"),
		*GetNameSafe(PawnData),
		*GetNameSafe(this),
		*GetClientServerContextString(this));
}

void ACrashPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, PawnData, SharedParams);
}
