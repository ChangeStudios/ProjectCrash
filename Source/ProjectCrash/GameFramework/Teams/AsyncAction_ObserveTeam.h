// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/CancellableAsyncAction.h"
#include "AsyncAction_ObserveTeam.generated.h"

class ICrashTeamAgentInterface;

/** Output pin for asynchronously observing team assignments and changes. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FTeamObservedAsyncSignature, bool, bTeamSet, int32, TeamID);

/**
 * Listens for changes to the specified agent's team, including initial assignment.
 */
UCLASS()
class PROJECTCRASH_API UAsyncAction_ObserveTeam : public UCancellableAsyncAction
{
	GENERATED_BODY()

	// Construction.

public:

	/**
	 * Listens for changes to the given agent's team, including initial assignment.
	 *
	 * This behaves similarly to ObserveTeamColors, but avoids the overhead of the display asset.
	 *
	 * @return bTeamSet		True if the agent was assigned to a valid team (i.e. not NoTeam).
	 * @return TeamId		The agent's current team ID. If the agent changed teams, this will be their new team ID.
	 */
	UFUNCTION(BlueprintCallable, Category = "Teams", Meta = (BlueprintInternalUseOnly = "true", Keywords = "watch, listen, register"))
	static UAsyncAction_ObserveTeam* ObserveTeam(UObject* TeamAgent);



	// Initialization.

public:

	/** Starts listening for changes to the agent's team. */
	virtual void Activate() override;

	/** Stops listening for changes to the agent's team. */
	virtual void SetReadyToDestroy() override;



	// Observation.

public:

	/** Called when the specified agent's team changes, including their initial team assignment. */
	UPROPERTY(BlueprintAssignable, DisplayName = "OnTeamChanged")
	FTeamObservedAsyncSignature TeamChangedDelegate;

private:

	/** Fires TeamChangedDelegate when the agent changes teams. */
	UFUNCTION()
	void OnAgentChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);



	// Internals.

private:

	/** The team agent being observed. */
	TWeakInterfacePtr<UObject> TeamAgentPtr;

	/** The agent interface of the agent being observed. Cached for convenience. */
	TWeakInterfacePtr<ICrashTeamAgentInterface> TeamAgentInterfacePtr;
};
