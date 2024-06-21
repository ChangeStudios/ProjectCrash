// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "GameFeatureAction.h"
#include "GameFeaturesSubsystem.h"
#include "GameFeatureAction_WorldActionBase.generated.h"

/**
 * Base class for game feature actions that affect the world. Adds the action to new and existing game worlds it's
 * activated.
 */
UCLASS(Abstract)
class UGameFeatureAction_WorldActionBase : public UGameFeatureAction
{
	GENERATED_BODY()

public:

	/** Adds the game action to any initialized worlds. Binds a delegate to add it to new worlds when their game
	 * instance is initialized. */
	virtual void OnGameFeatureActivating(FGameFeatureActivatingContext& Context) override;

	/** Clears callbacks. */
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;

private:

	/** Adds the action to the world when the game instance is initialized. */
	void HandleGameInstanceStart(UGameInstance* GameInstance, FGameFeatureStateChangeContext ChangeContext);

	/** Called when this action is added to a world. */
	virtual void AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext) PURE_VIRTUAL(UGameFeatureAction_WorldActionBase::AddToWorld,);

	/** Callbacks to when a game instances starts, so this action can be added to the world. */
	TMap<FGameFeatureStateChangeContext, FDelegateHandle> GameInstanceStartHandles;
};
