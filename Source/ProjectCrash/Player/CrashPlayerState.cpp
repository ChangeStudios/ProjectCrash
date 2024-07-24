// Copyright Samuel Reitich. All rights reserved.


#include "Player/CrashPlayerState.h"

#include "AbilitySystem/Abilities/CrashAbilitySet.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "Characters/PawnExtensionComponent.h"
#include "Characters/Data/PawnData.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GameFramework/CrashLogging.h"
#include "GameFramework/GameFeatures/GameFeatureAction_AddAbilities.h"
#include "GameFramework/GameModes/CrashGameMode.h"
#include "GameFramework/GameModes/CrashGameModeData.h"
#include "GameFramework/GameModes/GameModeManagerComponent.h"
#include "Net/UnrealNetwork.h"

ACrashPlayerState::ACrashPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Construct the ASC.
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UCrashAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	MinNetUpdateFrequency = 2.0f;
	NetUpdateFrequency = 100.0f;
}

void ACrashPlayerState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Initialize this player as the owner of its ASC.
	check(AbilitySystemComponent);
	AbilitySystemComponent->InitAbilityActorInfo(this, GetPawn());

	// Listen for the game mode to finish loading, so we can use it to initialize our pawn data.
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
		if (const UPawnData* NewPawnData = CrashGM->FindDefaultPawnDataForPlayer(GetOwningController()))
		{
			SetPawnData(NewPawnData);
		}
		else
		{
			UE_LOG(LogCrashGameMode, Error, TEXT("ACrashPlayerState::OnGameModeLoaded unable to find pawn data to initialize player state [%s]."), *GetNameSafe(this));
		}
	}
}

void ACrashPlayerState::OnDeactivated()
{
	UWorld* World = GetWorld();

	// By default, always destroy deactivated players.
	bool bDestroyDeactivatedPlayers = true;

	// Determine if deactivated players should be immediately destroyed.
	switch (GetPlayerConnectionType())
	{
		case EPlayerConnectionType::Player:
		case EPlayerConnectionType::InactivePlayer:
			// Check if the game mode wants to keep deactivated players alive instead of immediately destroying them.
			if (World && World->IsGameWorld())
			{
				AGameStateBase* GS = GetWorld()->GetGameState();
				check(GS);
				UGameModeManagerComponent* GameModeManagerComponent = GS->FindComponentByClass<UGameModeManagerComponent>();
				check(GameModeManagerComponent);
				if (GameModeManagerComponent->IsGameModeLoaded())
				{
					if (const UCrashGameModeData* GameModeData = GameModeManagerComponent->GetCurrentGameModeDataChecked())
					{
						bDestroyDeactivatedPlayers = GameModeData->bDestroyDeactivatedPlayers;
						break;
					}
				}
			}
		default:
			break;
	}

	SetPlayerConnectionType(EPlayerConnectionType::InactivePlayer);

	// Destroy this player state if deactivated players should be immediately destroyed.
	if (bDestroyDeactivatedPlayers)
	{
		Destroy();
	}
}

void ACrashPlayerState::OnReactivated()
{
	// Change this player's connection type to "Active" when they are reactivated.
	if (GetPlayerConnectionType() == EPlayerConnectionType::InactivePlayer)
	{
		SetPlayerConnectionType(EPlayerConnectionType::Player);
	}
}

void ACrashPlayerState::SetPlayerConnectionType(EPlayerConnectionType NewConnectionType)
{
	// Update this player's current connection type.
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ConnectionType, this);
	ConnectionType = NewConnectionType;
}

void ACrashPlayerState::SetPawnData(const UPawnData* InPawnData)
{
	// Pawn data can only be changed by the server.
	if (!HasAuthority())
	{
		return;
	}

	// Players cannot switch between active pawn data. Use ChangePawn to safely change pawn data during the game.
	if (PawnData)
	{
		UE_LOG(LogCrash, Error, TEXT("Tried to set pawn data [%s] on player state [%s]. This player already has valid pawn data: [%s]."),
			*GetNameSafe(InPawnData),
			*GetNameSafe(this),
			*GetNameSafe(PawnData));

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
			AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, &GrantedPawnDataAbilitySets.AddDefaulted_GetRef());
		}
	}

	/* Tell the modular game framework that we are ready to add abilities. This is used to add additional game
	 * mode-specific abilities via game feature actions. */
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, UGameFeatureAction_AddAbilities::NAME_AbilitiesReady);

	ForceNetUpdate();
}

void ACrashPlayerState::Server_ChangePawn_Implementation(const UPawnData* InPawnData)
{
	// Remove the ability sets granted by the old pawn data.
	if (AbilitySystemComponent)
	{
		for (auto GrantedAbilitySet : GrantedPawnDataAbilitySets)
		{
			GrantedAbilitySet.RemoveFromAbilitySystem(AbilitySystemComponent);
		}
	}
	GrantedPawnDataAbilitySets.Reset();

	// Clear PawnData so SetPawnData works properly.
	PawnData = nullptr;

	// Destroy the pawn.
	if (APawn* CurrentPawn = GetPawn())
	{
		CurrentPawn->DetachFromControllerPendingDestroy();
		CurrentPawn->SetLifeSpan(0.1f);
		CurrentPawn->SetActorHiddenInGame(true);
	}

	// Update this player's pawn data.
	SetPawnData(InPawnData);

	// Restart the player, destroying their old pawn and spawning a new one.
	AController* OwningController = GetPlayerController();
	ensure(OwningController);
	GetWorld()->GetAuthGameMode()->RestartPlayer(OwningController);
}

void ACrashPlayerState::OnRep_PawnData()
{
	UE_LOG(LogCrashGameMode, Log, TEXT("Set pawn data [%s] for player [%s]. (%s)"),
		*GetNameSafe(PawnData),
		*GetNameSafe(this),
		*GetClientServerContextString(this));
}

UAbilitySystemComponent* ACrashPlayerState::GetAbilitySystemComponent() const
{
	return GetCrashAbilitySystemComponent();
}

void ACrashPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ConnectionType, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, PawnData, SharedParams);
}
