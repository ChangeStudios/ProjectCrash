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



	// Team stats.

public:

	/** Adds the specified number of tags to the given team's tags. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Teams")
	void AddTeamTags(int32 TeamId, FGameplayTag Tag, int32 Count);

	/** Removes the specified number of tags from the given team's tags. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Teams")
	void RemoveTeamTags(int32 TeamId, FGameplayTag Tag, int32 Count);

	/** Returns how many of the specified tag a given team has. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Teams")
	int32 GetTeamTagCount(int32 TeamId, FGameplayTag Tag) const;

	/** Returns whether the given team has at least one of the specified tag. Tag must match exactly. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Teams")
	bool TeamHasTag(int32 TeamId, FGameplayTag Tag) const;



	// Team interactions.

public:

	/** Returns the alignment between the teams of two given objects. */
	UFUNCTION(BlueprintCallable, BlueprintPure = "false", Category = "Teams", Meta = (ExpandEnumAsExecs = "ReturnValue"))
	ETeamAlignment CompareTeams(const UObject* A, const UObject* B, int32& TeamA, int32& TeamB) const;

	/** Returns the alignment between the teams of two given objects. */
	ETeamAlignment CompareTeams(const UObject* A, const UObject* B) const;

	/**
	 * Returns whether a given instigator can damage a given target. Objects of different teams can always damage
	 * each other, while objects of the same team cannot. Objects with a team can damage objects without a team if
	 * that object has an ASC.
	 *
	 * If the instigator and target are the same, or have the same outer player state, bAllowDamageToSelf is returned.
	 */
	bool CanCauseDamage(const UObject* Instigator, const UObject* Target, bool bAllowDamageToSelf = false) const;

	/**
	 * Returns whether a given instigator can heal a given target. Objects of the same team can always heal each other,
	 * while objects of different teams cannot. Objects without a team cannot be healed.
	 *
	 * If the instigator and target are the same, or have the same outer player state, bAllowHealSelf is returned.
	 */
	bool CanCauseHealing(const UObject* Instigator, const UObject* Target, bool bAllowHealSelf = true) const;



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
