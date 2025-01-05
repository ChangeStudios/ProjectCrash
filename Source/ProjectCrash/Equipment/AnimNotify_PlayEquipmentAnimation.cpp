// Copyright Samuel Reitich. All rights reserved.


#include "Equipment/AnimNotify_PlayEquipmentAnimation.h"

#include "EquipmentActor.h"
#include "EquipmentComponent.h"
#include "EquipmentInstance.h"

UAnimNotify_PlayEquipmentAnimation::UAnimNotify_PlayEquipmentAnimation() :
	EquipmentPerspective(EEquipmentPerspective::FirstPerson)
{
#if WITH_EDITORONLY_DATA
	// Notify's default color in the editor.
	NotifyColor = FColor(255, 25, 150);
#endif
}

FString UAnimNotify_PlayEquipmentAnimation::GetNotifyName_Implementation() const
{
	if (AnimationTag.IsValid())
	{
		return ("Play " + AnimationTag.ToString());
	}

	return Super::GetNotifyName_Implementation();
}

void UAnimNotify_PlayEquipmentAnimation::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (!AnimationTag.IsValid())
	{
		return;
	}

	// Find this animation's owning pawn's equipment component, if it has one.
	const AActor* Owner = MeshComp->GetOwner();

	if (const APawn* OwnerAsPawn = (Owner ? Cast<APawn>(Owner) : nullptr))
	{
		if (const UEquipmentComponent* EquipmentComp = UEquipmentComponent::FindEquipmentComponent(OwnerAsPawn))
		{
			if (const UEquipmentInstance* CurrentEquipment = EquipmentComp->GetEquipment())
			{
				// Send the equipment animation to every active equipment actor.
				for (AEquipmentActor* EquipmentActor : CurrentEquipment->GetSpawnedActors())
				{
					EquipmentActor->ProcessEquipmentAnimation(AnimationTag, EquipmentPerspective);
				}
			}
		}
	}
}
