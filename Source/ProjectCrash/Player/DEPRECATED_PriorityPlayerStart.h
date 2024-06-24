// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "DEPRECATED_PriorityPlayerStart.generated.h"

/**
 * 
 */
UCLASS(NotBlueprintable, Deprecated)
class PROJECTCRASH_API ADEPRECATED_PriorityPlayerStart : public APlayerStart
{
	GENERATED_BODY()

	// Construction.
	
public:

	/** Default constructor. */
	ADEPRECATED_PriorityPlayerStart(const FObjectInitializer& ObjectInitializer);



	// Player start prioritization.

// Accessors.
public:

	/** Retrieves this player start's target team ID. */
	uint8 GetTargetTeamID() const { return TargetTeamID; }

	/** Retrieves this player start's player spawn priority. */
	uint8 GetPlayerSpawnPriority() const { return PlayerSpawnPriority; }

// Internals.
protected:

	/**
	 * The ID of the team to which this player start belongs. Players are spawned at the player starts corresponding to
	 * their team.
	 *
	 * This can be used to order team locations.. E.g. team 1 may want to spawn opposite team 2.
	 */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Player Start")
	uint8 TargetTeamID;

	/** The priority of this spawn within its team. LOWER values are prioritized. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Player Start")
	uint8 PlayerSpawnPriority;
};
