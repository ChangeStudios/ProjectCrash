// Copyright Samuel Reitich. All rights reserved.


#include "Equipment/AnimNotifyState_HideEquipment.h"

#include "EquipmentActor.h"
#include "EquipmentComponent.h"
#include "EquipmentInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CrashLogging.h"

UAnimNotifyState_HideEquipment::UAnimNotifyState_HideEquipment()
{
#if WITH_EDITORONLY_DATA
	// Equipment won't exist in the animation editor, so there's no point in firing this.
	bShouldFireInEditor = false;

	NotifyColor = FColor(75, 225, 75);
#endif
}

void UAnimNotifyState_HideEquipment::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	// Make sure mesh and owner (the actor with equipment) are valid.
	if (!IsValid(MeshComp) || !(IsValid(MeshComp->GetOwner())))
	{
		EQUIPMENT_LOG(Warning, TEXT("Attempted to begin a \"Hide Equipment\" notify state in animation [%s], but it does not have a valid owning actor."), *GetNameSafe(Animation));
		return;
	}

	TArray<USceneComponent*> Children;
	MeshComp->GetChildrenComponents(true, Children);
	for (USceneComponent* ChildComponent : Children)
	{
		if (UEquipmentMeshComponent* EquipmentComponent = Cast<UEquipmentMeshComponent>(ChildComponent))
		{
			// Filter for specific equipment if desired.
			if (FilterEquipment.Num() && !FilterEquipment.HasTagExact(EquipmentComponent->GetEquipmentTag()))
			{
				continue;
			}

			// TODO: Filter for ASC's current equipment.

			/* Use HiddenInGame instead of Visibility to maintain perspectives. Equipment actors' perspective-based
			 * visibility (e.g. hiding first-person actors when in third-person) is managed by Visibility. */
			EquipmentComponent->SetHiddenInGame(true, true);
		}
	}
}

void UAnimNotifyState_HideEquipment::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	
	// Make sure mesh and owner (the actor with equipment) are valid.
	if (!IsValid(MeshComp) || !(IsValid(MeshComp->GetOwner())))
	{
		EQUIPMENT_LOG(Warning, TEXT("Attempted to end a \"Hide Equipment\" notify state in animation [%s], but it does not have a valid owning actor."), *GetNameSafe(Animation));
		return;
	}

	// We repeat this process instead of caching what we've hidden in case our current equipment changes.
	TArray<USceneComponent*> Children;
	MeshComp->GetChildrenComponents(true, Children);
	for (USceneComponent* ChildComponent : Children)
	{
		if (UEquipmentMeshComponent* EquipmentComponent = Cast<UEquipmentMeshComponent>(ChildComponent))
		{
			if (FilterEquipment.Num() && !FilterEquipment.HasTagExact(EquipmentComponent->GetEquipmentTag()))
			{
				continue;
			}

			// TODO: Filter for ASC's current equipment.

			EquipmentComponent->SetHiddenInGame(false, true);
		}
	}
}