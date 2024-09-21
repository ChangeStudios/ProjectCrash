// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagAssetInterface.h"
#include "GameplayTagContainer.h"
#include "GenericTeamAgentInterface.h"
#include "../../../../../../../UE Source/UnrealEngine/Engine/Plugins/Editor/GameplayTagsEditor/Source/GameplayTagsEditor/Private/GameplayTagEditorUtilities.h"
#include "GameFramework/PlayerStart.h"
#include "CrashPlayerStart.generated.h"

/**
 * Describes how a potential pawn can fit within the location of a player start. Used to minimize collisions.
 */
enum class EPlayerStartLocationOccupancy : uint8
{
	// The pawn can fit completely in the start location.
	Empty,
	// The pawn cannot fit in the start location, but can be adjusted to fit.
	Partial,
	// The pawn cannot fit in the start location and cannot be adjusted to fit.
	Full
};



/**
 * A player start that can be tagged with various properties to customize its behavior depending on the game mode.
 *
 * Also contains logic to prevent collisions and close-proximity starts.
 */
UCLASS(ClassGroup = "Common", HideCategories = "Collision")
class PROJECTCRASH_API ACrashPlayerStart : public APlayerStart, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

	// Starting.

public:

	/** Returns whether the given controller's pawn could fit at this player start's location. */
	EPlayerStartLocationOccupancy GetLocationOccupancy(AController* const ControllerToFit) const;

	/** Whether this start has been claimed by a player, and should not be used again. */
	bool IsClaimed() const { return ClaimedController != nullptr; }

	/** Claims this start for the given player, preventing it from being used again for at least MinExpirationDuration
	 * seconds. */
	bool TryClaim(AController* ClaimingController);

protected:

	/** Continuously called to check if this player start is ready to be unclaimed. */
	void CheckUnclaimed();

	/** The player that currently claims this player start. Null if unclaimed. */
	UPROPERTY(Transient)
	TObjectPtr<AController> ClaimedController;

private:

	/** How long to wait after a player is started here to allow it to start a new one. */
	float MinExpirationDuration = 5.0f;

	/** Interval at which to check if this start can be used again. */
	float ExpirationCheckInterval = 0.5f;

	/** Timer for checking when this start should be unclaimed. */
	FTimerHandle ExpirationTimerHandle;



	// Start properties.

public:

	/** Returns the ID of the team to for which this player start should be used. May be NoTeam. */
	uint8 GetTeam() const { return TeamIndex; }

	/** Returns this player start's tags. */
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override { TagContainer = PlayerStartTags; }

protected:

	/** The team to which this player start belongs. Can be left unset. Used in game modes where players are spawned
	 * according to their team. */
	UPROPERTY(EditAnywhere, Category = "Player Start")
	uint8 TeamIndex = FGenericTeamId::NoTeam;

	/** Tags which can be queried and used to customize spawning behavior, depending on the game mode. */
	UPROPERTY(EditAnywhere, Category = "Player Start", Meta = (Categories = "GameMode.PlayerStart"))
	FGameplayTagContainer PlayerStartTags;
};
