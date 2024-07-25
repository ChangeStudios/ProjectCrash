// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TeamSubsystem.generated.h"

class ATeamInfo;
class ACrashPlayerState;

/**
 * Represents the relationship between two actors' teams.
 */
UENUM(BlueprintType)
enum class ETeamAlignment : uint8
{
	// Both actors are members of the same team.
	SameTeam,

	// The actors are members of opposing teams.
	DifferentTeams,

	// At least one actor is invalid or not part of any team.
	InvalidArgument
};



/**
 * Subsystem responsible for managing teams and their agents. Provides a centralized, globally accessible location for
 * storing team-related information, and acts as an interface for team management.
 */
UCLASS()
class PROJECTCRASH_API UTeamSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

	// Initialization.

public:

	/** Registers Team Cheats with the cheat manager. */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Unregisters CheatManagerRegistrationHandle from the cheat manager. */
	virtual void Deinitialize() override;


	
	// Team management

public:

	/** Attempts to register a new team. */
	bool RegisterTeam(ATeamInfo* TeamInfo);

	/** Attempts to unregister the given team, if it's registered with this subsystem. */
	bool UnregisterTeam(ATeamInfo* TeamInfo);

	/**
	 * Changes the given actor's team ID to the specified team.
	 * 
	 * The team will be changed via the actor's player state, if one can be found. Otherwise, the team is changed on
	 * the actor itself, if it implements CrashTeamAgentInterface.
	 *
	 * Can only be called with authority.
	 */
	bool ChangeTeamForActor(AActor* ActorToChange, int32 NewTeamIndex);



	// Team interactions.

public:

	//void CompareTeams(const UObject* A, const UObject* B);

	//void CompareTeams();

	//bool CanCauseDamage();

	//bool CanCauseHealing();



	// Visuals.

public:

	//void GetTeamDisplayAsset(int32 TeamId, int32 ViewerTeamId);



	// Utilities.

public:

	/** Attempts to find any team associated with the given object. Checks for interfaces, player states, etc. */
	int32 FindTeamFromObject(const UObject* Object) const;

	/** Blueprint-exposed version of FindTeamFromObject. */
	UFUNCTION(BlueprintCallable, BlueprintPure = "false", Category = "Teams", Meta = (ToolTip = "Attempts to find any team associated with the given object. Checks for interfaces, player states, etc.", Keywords = "get"))
	void FindTeamFromObject(const UObject* Object, bool& bIsOnTeam, int32& TeamId) const;

	/** Attempts to find any player state associated with the given actor. Looks for owning actors, pawns, etc. */
	const ACrashPlayerState* FindPlayerStateFromActor(const AActor* Actor) const;

	/** Returns whether a team exists with the specified ID. */
	UFUNCTION(BlueprintCallable, Category = "Teams")
	bool DoesTeamExist(int32 TeamId) const;

	/** Returns a sorted list of the teams currently in TeamMap. */
	UFUNCTION(BlueprintCallable, BlueprintPure = "false", Category = "Teams")
	TArray<int32> GetTeamIds() const;



	// Internals.

private:

	/** Internal map of registered team's IDs and their info actor. */
	UPROPERTY()
	TMap<int32, ATeamInfo*> TeamMap;

	/** Delegate fired to create Team Cheats when the cheat managed is created. */
	FDelegateHandle CheatManagerRegistrationHandle;
};
