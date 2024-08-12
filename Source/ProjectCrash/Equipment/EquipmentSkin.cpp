// Copyright Samuel Reitich. All rights reserved.


#include "Equipment/EquipmentSkin.h"

#include "EquipmentActor.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#define LOCTEXT_NAMESPACE "Equipment"

#if WITH_EDITOR
EDataValidationResult UEquipmentSkin::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

	{
		int32 EntryIndex = 0;
		for (const FEquipmentSkinActorInfo& ActorInfo : FirstPersonActorsToSpawn)
		{
			if (ActorInfo.ActorToSpawn == nullptr)
			{
				Result = EDataValidationResult::Invalid;
				Context.AddError(FText::Format(LOCTEXT("NullFirstPersonActorEntry", "First-person equipment actor at index {0} has a null class."), FText::AsNumber(EntryIndex)));
			}

			++EntryIndex;
		}
	}

	{
		int32 EntryIndex = 0;
		for (const FEquipmentSkinActorInfo& ActorInfo : ThirdPersonActorsToSpawn)
		{
			if (ActorInfo.ActorToSpawn == nullptr)
			{
				Result = EDataValidationResult::Invalid;
				Context.AddError(FText::Format(LOCTEXT("NullThirdPersonActorEntry", "Third-person equipment actor at index {0} has a null class."), FText::AsNumber(EntryIndex)));
			}

			++EntryIndex;
		}
	}

	return Result;
}
#endif // WITH_EDITOR

#undef LOCTEXT_NAMESPACE