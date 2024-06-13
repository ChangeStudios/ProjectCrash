// Copyright Samuel Reitich. All rights reserved.


#include "GameFramework/GameModes/CrashGameModeData.h"
#include "GameFeatureAction.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#define LOCTEXT_NAMESPACE "CrashSystem"

#if WITH_EDITOR
EDataValidationResult UCrashGameModeData::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

	// Validate this game mode's game feature actions.
	int32 EntryIndex = 0;
	for (const UGameFeatureAction* Action : Actions)
	{
		if (Action)
		{
			EDataValidationResult ChildResult = Action->IsDataValid(Context);
			Result = CombineDataValidationResults(Result, ChildResult);
		}
		else
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::Format(LOCTEXT("ActionEntryIsNull", "Null entry at index {0} in Actions"), FText::AsNumber(EntryIndex)));
		}

		++EntryIndex;
	}

	return Result;
}
#endif // WITH_EDITOR

void UCrashGameModeData::UpdateAssetBundleData()
{
	Super::UpdateAssetBundleData();

	// Update this game mode's actions' bundles when the game mode asset itself is updated.
	for (UGameFeatureAction* Action : Actions)
	{
		if (Action)
		{
			Action->AddAdditionalAssetBundleData(AssetBundleData);
		}
	}
}

#undef LOCTEXT_NAMESPACE