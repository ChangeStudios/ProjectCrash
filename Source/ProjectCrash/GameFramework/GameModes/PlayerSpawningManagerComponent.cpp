// Copyright Samuel Reitich. All rights reserved.


#include "GameFramework/GameModes/PlayerSpawningManagerComponent.h"

#include "AbilitySystemGlobals.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/SpectatorPawn.h"
#include "Player/CrashPlayerController.h"
#include "Player/CrashPlayerState.h"

UPlayerSpawningManagerComponent::UPlayerSpawningManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(false);
	bAutoRegister = true;
	bAutoActivate = true;
	bWantsInitializeComponent = true;
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerSpawningManagerComponent::InitializeComponent()
{
	Super::InitializeComponent();

	// Listen for levels loading in to cache any player starts they might have.
	FWorldDelegates::LevelAddedToWorld.AddUObject(this, &ThisClass::OnLevelAdded);

	// TODO: Cache player starts.
}

void UPlayerSpawningManagerComponent::OnLevelAdded(ULevel* InLevel, UWorld* InWorld)
{
	// TODO: Cache any new player starts.
}