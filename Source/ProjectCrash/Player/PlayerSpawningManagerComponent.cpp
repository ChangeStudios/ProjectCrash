// Copyright Samuel Reitich. All rights reserved.


#include "Player/PlayerSpawningManagerComponent.h"

#include "CrashGameplayTags.h"
#include "CrashPlayerStart.h"
#include "EngineUtils.h"
#include "Engine/PlayerStartPIE.h"
#include "GameFramework/PlayerState.h"

UPlayerSpawningManagerComponent::UPlayerSpawningManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(false);
	bAutoRegister = true;
	bAutoActivate = true;
	bWantsInitializeComponent = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bAllowTickOnDedicatedServer = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
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
			PlayerStarts_Internal.Add(CrashPlayerStart);
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
				PlayerStarts_Internal.AddUnique(CrashPlayerStart);
			}
		}
	}
}

AActor* UPlayerSpawningManagerComponent::ChoosePlayerStart(AController* Player)
{
	if (Player)
	{
#if WITH_EDITOR
		// Override the player start for "Play From Here" requests.
		if (APlayerStart* PlayerStart = FindPlayFromHereStart(Player))
		{
			return PlayerStart;
		}
#endif // WITH_EDITOR

		// Validate cached player starts (they're weak pointers).
		TArray<ACrashPlayerStart*> ValidStarts;
		for (auto StartIt = PlayerStarts_Internal.CreateIterator(); StartIt; ++StartIt)
		{
			if (ACrashPlayerStart* CrashPlayerStart = StartIt->Get())
			{
				ValidStarts.Add(CrashPlayerStart);
			}
			else
			{
				StartIt.RemoveCurrent();
			}
		}

		// Spawn spectators at dedicated player starts.
		if (APlayerState* PlayerState = Player->GetPlayerState<APlayerState>())
		{
			if (PlayerState->IsOnlyASpectator())
			{
				// Try to find a player start with the "Spectator" tag.
				if (APlayerStart* SpectatorStart = FindFirstStartWithTag(CrashGameplayTags::TAG_GameMode_PlayerStart_Spectator))
				{
					return SpectatorStart;
				}
				// Spawn the spectator at a random player start if a dedicated one can't be found.
				else
				{
					if (!ValidStarts.IsEmpty())
					{
						return ValidStarts[FMath::RandRange(0, ValidStarts.Num() - 1)];
					}
				}

				return nullptr;
			}
		}

		// Remove spectator starts if we're not spawning a spectator.
		ValidStarts.RemoveAll( [](const ACrashPlayerStart* PlayerStart) { return PlayerStart->HasMatchingGameplayTag(CrashGameplayTags::TAG_GameMode_PlayerStart_Spectator); });

		// Try to find the best start for this player.
		AActor* PlayerStart = FindBestPlayerStart(Player, ValidStarts);

		// Fall back to any available player start.
		if (!PlayerStart)
		{
			PlayerStart = FindFirstUnoccupiedPlayerStart(Player, ValidStarts);
		}

		// Claim the start for the player.
		if (ACrashPlayerStart* CrashPlayerStart = Cast<ACrashPlayerStart>(PlayerStart))
		{
			CrashPlayerStart->TryClaim(Player);
		}

		return PlayerStart;
	}

	return nullptr;
}

#if WITH_EDITOR
APlayerStart* UPlayerSpawningManagerComponent::FindPlayFromHereStart(AController* Player)
{
	// Only "Play From Here" for players.
	if (Player->IsA<APlayerController>())
	{
		if (UWorld* World = GetWorld())
		{
			for (TActorIterator<APlayerStart> It(World); It; ++It)
			{
				if (APlayerStart* PlayerStart = *It)
				{
					// Always use the first "Play From Here" start.
					if (PlayerStart->IsA<APlayerStartPIE>())
					{
						return PlayerStart;
					}
				}
			}
		}
	}

	return nullptr;
}
#endif // WITH_EDITOR

APlayerStart* UPlayerSpawningManagerComponent::FindFirstStartWithTag(FGameplayTag Tag)
{
	// Search all cached player starts for one with the given tag.
	for (auto StartIt = PlayerStarts_Internal.CreateIterator(); StartIt; ++StartIt)
	{
		if (ACrashPlayerStart* Start = StartIt->Get())
		{
			if (Start->HasMatchingGameplayTag(Tag))
			{
				return Start;
			}
		}
	}

	return nullptr;
}

APlayerStart* UPlayerSpawningManagerComponent::FindFirstUnoccupiedPlayerStart(AController* Player, TArray<ACrashPlayerStart*>& PlayerStarts) const
{
	if (Player)
	{
		// The first occupied player start, which we'll fall back to if no unoccupied starts are found.
		ACrashPlayerStart* FallbackStart = nullptr;

		for (ACrashPlayerStart* PlayerStart : PlayerStarts)
		{
			switch (PlayerStart->GetLocationOccupancy(Player))
			{
				// Return the first unoccupied start.
				case EPlayerStartLocationOccupancy::Empty:
					return PlayerStart;
				// Cache the first occupied start to fall back to.
				case EPlayerStartLocationOccupancy::Partial:
					if (FallbackStart == nullptr)
						FallbackStart = PlayerStart;
					break;
				default:
					break;
			}
		}

		// If no unoccupied starts were found, try to fall back to the first occupied start.
		if (FallbackStart)
		{
			return FallbackStart;
		}
	}

	return nullptr;
}
