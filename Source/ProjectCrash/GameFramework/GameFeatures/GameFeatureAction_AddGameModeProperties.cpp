// Copyright Samuel Reitich. All rights reserved.


#include "GameFramework/GameFeatures/GameFeatureAction_AddGameModeProperties.h"

#include "GameFramework/GameModes/GameModePropertySubsystem.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#define LOCTEXT_NAMESPACE "GameFeatures"

void UGameFeatureAction_AddGameModeProperties::OnGameFeatureActivating(FGameFeatureActivatingContext& Context)
{
	// Create or reset context data for the given context.
	FPerContextData& ActiveData = ContextData.FindOrAdd(Context);
	if (!ensureAlways(ActiveData.RegisteredProperties.IsEmpty()))
	{
		Reset(ActiveData);
	}

	Super::OnGameFeatureActivating(Context);
}

void UGameFeatureAction_AddGameModeProperties::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context);

	// Reset the context data for the given context, unregistering the game mode properties added by this action.
	FPerContextData* ActiveData = ContextData.Find(Context);
	if (ensure(ActiveData))
	{
		Reset(*ActiveData);
	}
}

void UGameFeatureAction_AddGameModeProperties::AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext)
{
	UWorld* World = WorldContext.World();
	FPerContextData& ActiveData = ContextData.FindOrAdd(ChangeContext);

	// Register this action's game mode properties with the world's game mode property subsystem.
	if ((World != nullptr) && (World->IsGameWorld()))
	{
		if (UGameModePropertySubsystem* GameModePropertySubsystem = World->GetSubsystem<UGameModePropertySubsystem>())
		{
			ActiveData.GameModePropertySubsystem = GameModePropertySubsystem;

			for (const FGameFeaturePropertiesEntry& Entry : GameModePropertiesList)
			{
				if (Entry.Property.IsValid())
				{
					if (GameModePropertySubsystem->AddGameModeProperty(Entry.Property, Entry.Value))
					{
						ActiveData.RegisteredProperties.Add(Entry.Property);
					}
				}
			}
		}
	}
}

void UGameFeatureAction_AddGameModeProperties::Reset(FPerContextData& ActiveData)
{
	if (UGameModePropertySubsystem* GameModePropertySubsystem = ActiveData.GameModePropertySubsystem.Get())
	{
		// Remove the game mode properties registered in the given context by this action.
		for (const FGameplayTag& Property : ActiveData.RegisteredProperties)
		{
			GameModePropertySubsystem->RemoveGameModeProperty(Property);
		}

		// Clear the cached properties.
		ActiveData.RegisteredProperties.Empty();
	}
}

#if WITH_EDITOR
EDataValidationResult UGameFeatureAction_AddGameModeProperties::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

	// Make sure this action actually does something (has properties to add).
	if (GameModePropertiesList.IsEmpty())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(LOCTEXT("ActionAddsNothing", "\"Add Game Mode Properties\" action will do nothing: No game mode properties are defined."));
	}

	int32 EntryIndex = 0;
	for (const FGameFeaturePropertiesEntry& Entry : GameModePropertiesList)
	{
		// Validate the property tag.
		if (!Entry.Property.IsValid())
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::Format(LOCTEXT("EntryHasNullTag", "Invalid tag at index [{0}] in \"Add Game Mode Properties\" action."), FText::AsNumber(EntryIndex)));
		}

		// Make sure there are no duplicate game mode properties.
		for (int32 DuplicateIndex = EntryIndex + 1; DuplicateIndex < GameModePropertiesList.Num(); ++DuplicateIndex)
		{
			if (Entry.Property.MatchesTagExact(GameModePropertiesList[DuplicateIndex].Property))
			{
				Result = EDataValidationResult::Invalid;
				Context.AddError(FText::Format(LOCTEXT("DuplicatePropertyEntry", "Duplicate game mode property entry for [{0}]. Index [{1}]: {2}. Index [{3}]: {4}. Duplicate game mode properties are not allowed."),
					FText::FromString(Entry.Property.ToString()),
					FText::AsNumber(EntryIndex),
					FText::AsNumber(Entry.Value),
					FText::AsNumber(DuplicateIndex),
					FText::AsNumber(GameModePropertiesList[DuplicateIndex].Value)
				));
			}
		}

		EntryIndex++;
	}

	return Result;
}
#endif // WITH_EDITOR

#undef LOCTEXT_NAMESPACE