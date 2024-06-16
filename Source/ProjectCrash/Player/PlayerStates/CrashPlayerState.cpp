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
const FName ACrashPlayerState::NAME_ActorFeatureName("CrashPlayerState");

void ACrashPlayerState::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	// Register this actor as a feature with the initialization state framework.
	RegisterInitStateFeature();
}

void ACrashPlayerState::BeginPlay()
{
	Super::BeginPlay();

	// Start listening for initialization state changes on this actor.
	BindOnActorInitStateChanged(NAME_None, FGameplayTag(), false);

	// Initialize this actor's initialization state.
	ensure(TryToChangeInitState(STATE_WAITING_FOR_DATA));
	CheckDefaultInitialization();
}

void ACrashPlayerState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Unregister this actor as an initialization state feature.
	UnregisterInitStateFeature();
	
	Super::EndPlay(EndPlayReason);
}

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
		GameModeManagerComponent->CallOrRegister_OnGameModeLoaded(FCrashGameModeLoadedSignature::FDelegate::CreateUObject(this, &ThisClass::OnGameModeLoaded));
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

	// Try to progress our initialization state.
	CheckDefaultInitialization();
}

bool ACrashPlayerState::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
	check(Manager);

	// We can always transition to our initial state when our current state hasn't been initialized yet.
	if (!CurrentState.IsValid() && DesiredState == STATE_WAITING_FOR_DATA)
	{
		return true;
	}
	// Transition to Initializing when (1) our pawn data has been initialized and (2) all of our components are ready.
	else if (CurrentState == STATE_WAITING_FOR_DATA && DesiredState == STATE_INITIALIZING)
	{
		// Pawn data must be set before we can initialize this player.
		if (!PawnData)
		{
			return false;
		}

		// All of this actor's components must be Initializing. 
		return Manager->HaveAllFeaturesReachedInitState(const_cast<ACrashPlayerState*>(this), STATE_INITIALIZING, GetFeatureName());
	}
	/* Transition to GameplayReady when (1) we possess a pawn, (2) all of our components are ready, and (3) all of our
	 * pawn's components are ready, via the pawn extension component. */
	else if (CurrentState == STATE_INITIALIZING && DesiredState == STATE_GAMEPLAY_READY)
	{
		// We must possess a pawn for gameplay to begin.
		if (!GetPawn())
		{
			return false;
		}

		// All of this actor's components must be GameplayReady.
		if (!Manager->HaveAllFeaturesReachedInitState(const_cast<ACrashPlayerState*>(this), STATE_GAMEPLAY_READY, GetFeatureName()))
		{
			return false;
		}

		// TODO: Find PawnExtensionComponent on pawn and check its progress.

		return true;
	}

	return false;
}

void ACrashPlayerState::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	// If another feature has changed init states (e.g. one of our components), check if we should too.
	if (Params.FeatureName != NAME_ActorFeatureName)
	{
		CheckDefaultInitialization();
	}
}

void ACrashPlayerState::CheckDefaultInitialization()
{
	// Before checking our progress, try progressing any other features we might depend on.
	CheckDefaultInitializationForImplementers();
	ContinueInitStateChain(CrashGameplayTags::StateChain);
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
