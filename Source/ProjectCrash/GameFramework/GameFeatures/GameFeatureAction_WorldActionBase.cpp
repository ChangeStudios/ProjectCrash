// Copyright Samuel Reitich. All rights reserved.


#include "GameFramework/GameFeatures/GameFeatureAction_WorldActionBase.h"

void UGameFeatureAction_WorldActionBase::OnGameFeatureActivating(FGameFeatureActivatingContext& Context)
{
	// Bind to when the world is initialized to add this game feature.
	GameInstanceStartHandles.FindOrAdd(Context) = FWorldDelegates::OnStartGameInstance.AddUObject(this,&UGameFeatureAction_WorldActionBase::HandleGameInstanceStart, FGameFeatureStateChangeContext(Context));

	// Add to any worlds with associated game instances that have already been initialized.
	for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
	{
		if (Context.ShouldApplyToWorldContext(WorldContext))
		{
			AddToWorld(WorldContext, Context);
		}
	}
}

void UGameFeatureAction_WorldActionBase::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	// Remove the callback to this world's initialization.
	FDelegateHandle* FoundHandle = GameInstanceStartHandles.Find(Context);
	if (ensure(FoundHandle))
	{
		FWorldDelegates::OnStartGameInstance.Remove(*FoundHandle);
	}
}

void UGameFeatureAction_WorldActionBase::HandleGameInstanceStart(UGameInstance* GameInstance, FGameFeatureStateChangeContext ChangeContext)
{
	// Add the game feature to the world when it's initialized.
	if (FWorldContext* WorldContext = GameInstance->GetWorldContext())
	{
		if (ChangeContext.ShouldApplyToWorldContext(*WorldContext))
		{
			AddToWorld(*WorldContext, ChangeContext);
		}
	}
}
