// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TeamSubsystem.generated.h"

class ATeamInfo;
class ACrashPlayerState;

/** Delegate for when this team's display asset changes. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTeamDisplayAssetChangedSignature, const UTeamDisplayAsset*, DisplayAsset);

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
 * Represents a team present in the team subsystem. Responsible for managing the team's information actor and display
 * assets.
 *
 * This structure is used, rather than a reference to the team info actor itself (which has its own references to all
 * of this data) to be able to handle and facilitate data before the team info actor has been replicated. For example,
 * this allows other systems to start listening for team display assets to be set before the team info actor has been
 * replicated.
 *
 * TODO: Replace ATeamInfo in TeamMap with this.
 */
USTRUCT()
struct FTeamTrackingInfo
{
	GENERATED_BODY()

public:

	/** */
	UPROPERTY()
	TObjectPtr<ATeamInfo> TeamInfo = nullptr;

	UPROPERTY()
	TObjectPtr<UTeamDisplayAsset> FriendlyDisplayAsset = nullptr;

	UPROPERTY()
	TObjectPtr<UTeamDisplayAsset> TeamDisplayAsset = nullptr;

	UPROPERTY()
	FTeamDisplayAssetChangedSignature TeamDisplayAssetChangedDelegate;

public:

	/** Sets this team's info actor. This can also be called after a team's display assets change to refresh them. */
	void SetTeamInfo(ATeamInfo* NewTeamInfo);
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



	// Display assets.

public:

	/** Returns the display asset that should be used to view the target team from the perspective of the given viewer's
	 * team. If the viewer and target are on the same team, the team's "friendly display asset" will be used, if one
	 * exists. Otherwise, the team's normal display asset with be used. */
	UFUNCTION(BlueprintCallable, Category = "Teams")
	UTeamDisplayAsset* GetTeamDisplayAsset(int32 TeamId, int32 ViewerTeamId);

	/** Fires the TeamDisplayAssetChangedDelegate for any teams using the given display asset when the asset is
	 * modified. Used for modifying display assets in PIE. */
	void NotifyTeamDisplayAssetModified(UTeamDisplayAsset* ModifiedAsset);

	/** Returns the TeamDisplayAssetChangedDelegate for the team with the given ID. */
	FTeamDisplayAssetChangedSignature* GetTeamDisplayAssetChangedDelegate(int32 TeamId);



	// Utilities.

public:

	/** Attempts to find any team associated with the given object. Returns INDEX_NONE if no team could be found, or if
	 * the object is explicitly assigned to NoTeam. */
	int32 FindTeamFromObject(const UObject* Object) const;

	/** Attempts to find any player state associated with the given actor. */
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
