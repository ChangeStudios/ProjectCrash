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
 * Also listens for team assignments and changes for the specified agent. This allows the agent to automatically update
 * their colors when changing teams, in addition to when their current team's colors change.
 */
UCLASS()
class PROJECTCRASH_API UAsyncAction_ObserveTeamColors : public UCancellableAsyncAction
{
	GENERATED_BODY()

	// Construction.

public:

	/**
	 * Listens for changes to the given agent's team and their team's colors.
	 *
	 * @param  bLocalViewer		If true, DisplayAsset will be from the perspective of the local player. Otherwise, the
	 *							team's normal display asset will always be used.
	 * @return bTeamSet			True if the agent's team was assigned for the first time or if it was changed.
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
	void OnTeamDisplayAssetChanged(const UTeamDisplayAsset* NewDisplayAsset);



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
};
