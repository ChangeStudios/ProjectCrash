// Copyright Samuel Reitich. All rights reserved.


#include "Equipment/AnimNotify_PlayEquipmentAnimation.h"

#include "EquipmentActor.h"
#include "EquipmentComponent.h"
#include "EquipmentInstance.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif	// WITH_EDITOR

#define LOCTEXT_NAMESPACE "EquipmentAnimationNotify"

UAnimNotify_PlayEquipmentAnimation::UAnimNotify_PlayEquipmentAnimation() :
	EquipmentAnimation(nullptr),
	EquipmentPerspective(EEquipmentPerspective::FirstPerson)
{
#if WITH_EDITORONLY_DATA
	// Notify's default color in the editor.
	NotifyColor = FColor(255, 25, 150);
#endif
}

FString UAnimNotify_PlayEquipmentAnimation::GetNotifyName_Implementation() const
{
	if (IsValid(EquipmentAnimation))
	{
		return ("Play " + EquipmentAnimation->GetName());
	}

	return "(Unset) Play Equipment Animation";
}

void UAnimNotify_PlayEquipmentAnimation::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (!IsValid(EquipmentAnimation) || !Equipment.IsValid())
	{
		return;
	}

	const AActor* Owner = MeshComp->GetOwner();
	TArray<UEquipmentMeshComponent*> EquipmentComponents;
	Owner->GetComponents<UEquipmentMeshComponent>(EquipmentComponents);

	// Play the specified animation on each equipment mesh in the owning actor with a matching tag and perspective.
	for (UEquipmentMeshComponent* EquipmentComponent : EquipmentComponents)
	{
		if (EquipmentComponent->GetEquipmentTag() == Equipment &&
			EquipmentComponent->GetEquipmentPerspective() == EquipmentPerspective)
		{
			EquipmentComponent->PlayAnimation(EquipmentAnimation, EquipmentAnimation->bLoop);
		}
	}
}

#if WITH_EDITOR
EDataValidationResult UAnimNotify_PlayEquipmentAnimation::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(EDataValidationResult::Valid, Super::IsDataValid(Context));

	// Make sure we've selected an animation.
	if (!IsValid(EquipmentAnimation))
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(LOCTEXT("NoAnimationSpecified", "\"Play Equipment Animation\" notify does not specify an animation to play."));
	}

	// Make sure we've selected an equipment tag.
	if (!Equipment.IsValid())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(LOCTEXT("NoEquipmentSpecified", "\"Play Equipment Animation\" notify does not specify which equipment to play the animation on."));
	}

	return Result;
}
#endif // WITH_EDITOR
