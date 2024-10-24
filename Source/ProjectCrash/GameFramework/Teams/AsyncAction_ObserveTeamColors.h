// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TeamDisplayAsset.h"
#include "Engine/CancellableAsyncAction.h"
#include "AsyncAction_ObserveTeamColors.generated.h"

class ICrashTeamAgentInterface;
class UTeamDisplayAsset;

/** Output pin for asynchronously observing team colors. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FTeamColorObservedAsyncSignature, bool, bTeamSet, int32, TeamID, const UTeamDisplayAsset*, DisplayAsset);

/**
 * Listens for team color changes for a specified agent (changes to their team display assets).
 *
 * Also listens for changes to the specified agent's team, including initial assignment. This allows the agent to
 * automatically update their colors when changing teams, in addition to when their current team's colors change.
 */
UCLASS()
class PROJECTCRASH_API UAsyncAction_ObserveTeamColors : public UCancellableAsyncAction
{
	GENERATED_BODY()

	// Construction.

public:

	/**
	 * Listens for changes to the given agent's team (including initial assignment) and their team's colors.
	 *
	 * @param  bLocalViewer		If true, DisplayAsset will be from the perspective of the local player. Otherwise, the
	 *							team's normal display asset will always be used.
	 * @return bTeamSet			True if the agent was assigned to a valid team (i.e. not NoTeam).
	 * @return TeamId			The agent's current team ID. If the agent changed teams, this will be their new team ID.
	 * @return DisplayAsset		The agent's current team's effective display asset. If the agent changed teams, this
	 *							will be their new team's display asset.
	 */
	UFUNCTION(BlueprintCallable, Category = "Teams", Meta = (BlueprintInternalUseOnly = "true", Keywords = "watch, listen, register"))
	static UAsyncAction_ObserveTeamColors* ObserveTeamColors(UObject* TeamAgent, bool bLocalViewer = true);



	// Initialization.

public:

	/** Starts listening for changes to the agent's team or their team's display assets. */
	virtual void Activate() override;

	/** Stops listening for changes to the agent's team or their team's display assets. */
	virtual void SetReadyToDestroy() override;



	// Observation.

public:

	/** Called when the agent's current team's colors change. Also called when the agent's team changes (including
	 * their initial team assignment) since this will likely also change the agent's team colors. */
	UPROPERTY(BlueprintAssignable, DisplayName = "OnTeamColorChanged")
	FTeamColorObservedAsyncSignature TeamColorChangedDelegate;

private:

	/** Fires the TeamColorChangedDelegate with the observed agent's current state. */
	void BroadcastChange(int32 NewTeam, const UTeamDisplayAsset* DisplayAsset);

	/** Broadcasts changes to the observed agent's team. */
	UFUNCTION()
	void OnAgentChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);

	/** Broadcasts changes to the observed agent's team's colors. */
	UFUNCTION()
	void OnTeamDisplayAssetChanged(int32 TeamId);

	/** Refreshes the observed agent's team display asset, relative to the viewer's new team. */
	UFUNCTION()
	void OnViewerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);



	// Internals.

private:

	/** Whether to view the observed agent's team from the perspective of the local player. */
	bool bLocalViewer;

	/** The team agent being observed. */
	TWeakObjectPtr<UObject> TeamAgentPtr;

	/** The agent interface of the agent being observed. Cached for convenience. */
	TWeakInterfacePtr<ICrashTeamAgentInterface> TeamAgentInterfacePtr;

	/** The last team ID which was broadcast with a team or team color change. */
	int32 LastBroadcastTeamId = INDEX_NONE;



	// Utils.

private:

	/** Retrieves the relevant display asset for the observed team. If bLocalViewer is true, uses the local player's
	 * team to determine which display asset to use. Otherwise, uses the team's normal display asset. */
	UTeamDisplayAsset* GetRelevantDisplayAsset(int32 TeamId);

	/** Retrieves the local viewer for this action. Returns the local player controller, if valid, if the bLocalViewer
	 * is true. Returns nullptr otherwise. */
	UObject* GetViewer(UObject* WorldContextObject);
};
