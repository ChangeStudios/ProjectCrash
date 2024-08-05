// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagStack.h"
#include "TeamSubsystem.h"
#include "GameFramework/Info.h"
#include "TeamInfo.generated.h"

class UTeamDisplayAsset;
class UTeamCreationComponent;
class UTeamSubsystem;

/**
 * The tangible representation of a team during gameplay. Contains an identifying team ID and stores replicated runtime
 * data related to the team with stacks of gameplay tags.
 */
UCLASS()
class PROJECTCRASH_API ATeamInfo : public AInfo
{
	GENERATED_BODY()
	
	// The team creation component is the only class that should be setting team info data.
	friend UTeamCreationComponent;

	// Construction.

public:

	/** Default constructor. */
	ATeamInfo(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());



	// Initialization.

public:

	/** Attempts to register this team when it's created. */
	virtual void BeginPlay() override;

	/** Unregisters this team when destroyed. */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:

	/** Registers this team with the team subsystem if it has a valid ID. */
	void TryRegisterWithTeamSubsystem();



	// Team ID.

public:

	/** Returns this team's ID. */
	int32 GetTeamId() const { return TeamId; }

private:

	/** Sets this team's ID and registers it with the team subsystem on the server. Can only be called once, to
	 * initialize the ID. Can only be called with authority. */
	void SetTeamId(int32 NewTeamId);

	/** This team's internal ID. */
	UPROPERTY(ReplicatedUsing = OnRep_TeamId)
	int32 TeamId;

	/** Registers this team with the team subsystem when it's assigned a valid ID on clients. */
	UFUNCTION()
	void OnRep_TeamId();



	// Team info.

public:

	/** Collection of gameplay tag stacks that can be used to track runtime team stats (e.g. a team's score). */
	UPROPERTY(Replicated)
	FGameplayTagStackContainer TeamTags;



	// Display assets.

public:

	/** Returns the display asset used when a member of this team views it. */
	UTeamDisplayAsset* GetFriendlyDisplayAsset() const { return FriendlyDisplayAsset; }

	/** Returns the display asset used by this team when viewed by someone on another team, or by someone without a
	 * team (e.g. spectators). */
	UTeamDisplayAsset* GetTeamDisplayAsset() const { return TeamDisplayAsset; }

// Friendly display asset.
protected:

	/** Updates this team's friendly display asset. */
	void SetFriendlyDisplayAsset(TObjectPtr<UTeamDisplayAsset> NewDisplayAsset);

	/** The display asset used by this team when viewed by a member of this team. If unset, TeamDisplayAsset will be
	 * used instead. */
	UPROPERTY(ReplicatedUsing = OnRep_FriendlyDisplayAsset)
	TObjectPtr<UTeamDisplayAsset> FriendlyDisplayAsset;

	/** Broadcasts this team's friendly display asset change. */
	UFUNCTION()
	void OnRep_FriendlyDisplayAsset(UTeamDisplayAsset* OldDisplayAsset);

// Team display asset.
protected:

	/** Updates this team's display asset. */
	void SetTeamDisplayAsset(TObjectPtr<UTeamDisplayAsset> NewDisplayAsset);

	/** The display asset used by this team when viewed by someone on another team or by someone without a team (e.g.
	 * spectators). */
	UPROPERTY(ReplicatedUsing = OnRep_TeamDisplayAsset)
	TObjectPtr<UTeamDisplayAsset> TeamDisplayAsset;

	/** Broadcasts this team's display asset change. */
	UFUNCTION()
	void OnRep_TeamDisplayAsset(UTeamDisplayAsset* OldDisplayAsset);
};
