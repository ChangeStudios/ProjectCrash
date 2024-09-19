// Copyright Samuel Reitich. All rights reserved.


#include "Player/PlayerSpawningManagerComponent.h"

#include "CrashPlayerStart.h"
#include "EngineUtils.h"

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

	// Cache all player starts in the level.
	for (TActorIterator<ACrashPlayerStart> It(GetWorld()); It; ++It)
	{
		if (ACrashPlayerStart* CrashPlayerStart = *It)
		{
			PlayerStarts.Add(CrashPlayerStart);
		}
	}
}

void UPlayerSpawningManagerComponent::OnLevelAdded(ULevel* InLevel, UWorld* InWorld)
{
	// Cache any new player starts.
	if (InWorld == GetWorld())
	{
		for (AActor* Actor : InLevel->Actors)
		{
			if (ACrashPlayerStart* CrashPlayerStart = Cast<ACrashPlayerStart>(Actor))
			{
				PlayerStarts.AddUnique(CrashPlayerStart);
			}
		}
	}
}

AActor* UPlayerSpawningManagerComponent::ChoosePlayerStart(AController* Player)
{
}

APlayerStart* UPlayerSpawningManagerComponent::FindFirstUnoccupiedPlayerStart(AController* Player) const
{
}
