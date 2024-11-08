// Copyright Samuel Reitich. All rights reserved.


#include "Equipment/AnimNotifyState_HideEquipment.h"

#include "EquipmentActor.h"
#include "EquipmentComponent.h"
#include "EquipmentInstance.h"
#include "GameFramework/CrashLogging.h"

UAnimNotifyState_HideEquipment::UAnimNotifyState_HideEquipment()
{
#if WITH_EDITORONLY_DATA
	// Notify's default color in the editor.
	NotifyColor = FColor(75, 225, 75);
#endif

	// We won't have valid equipment in the editor, so there's no point in firing this.
	bShouldFireInEditor = false;
}

void UAnimNotifyState_HideEquipment::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	// Make sure mesh and owning actor (the pawn with equipment) are valid.
	if (!IsValid(MeshComp) || !(IsValid(MeshComp->GetOwner())))
	{
		EQUIPMENT_LOG(Warning, TEXT("Attempted to begin a \"Hide Equipment\" notify state in animation [%s], but it does not have a valid owning actor."), *GetNameSafe(Animation));
		return;
	}

	// Retrieve the equipment component from the mesh's owning actor, which should be a pawn.
	APawn* OwnerAsPawn = Cast<APawn>(MeshComp->GetOwner());
	UEquipmentComponent* EquipmentComp = (OwnerAsPawn ? UEquipmentComponent::FindEquipmentComponent(OwnerAsPawn) : nullptr);

	if (!IsValid(EquipmentComp))
	{
		EQUIPMENT_LOG(Warning, TEXT("Attempted to begin a \"Hide Equipment\" notify state in animation [%s], but its owning actor [%s] does not have an equipment component."), *GetNameSafe(Animation), *GetNameSafe(MeshComp->GetOwner()));
		return;
	}

	// Iterate through each equipment actor for the current equipment and hide it.
	if (const UEquipmentInstance* EquipmentInstance = EquipmentComp->GetEquipment())
	{
		for (const AEquipmentActor* EquipmentActor : EquipmentInstance->GetSpawnedActors())
		{
			if (IsValid(EquipmentActor))
			{
				if (USceneComponent* Root = EquipmentActor->GetRootComponent())
				{
					/* Only hide equipment attached to the owning mesh, so third-person animations don't hide first-person 
					 * equipment or vice versa. */
					if (MeshComp->GetAttachChildren().Contains(Root))
					{
						/* Use HiddenInGame instead of Visibility to maintain perspectives. Equipment actors' perspective-based
						 * visibility (e.g. hiding first-person actors when in third-person) is managed by Visibility. */
						Root->SetHiddenInGame(true, true);
					}
				}
			}
		}
	}
}

void UAnimNotifyState_HideEquipment::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	
	// Make sure mesh and owning actor (the pawn with equipment) are valid.
	if (!IsValid(MeshComp) || !(IsValid(MeshComp->GetOwner())))
	{
		EQUIPMENT_LOG(Warning, TEXT("Attempted to end a \"Hide Equipment\" notify state in animation [%s], but it does not have a valid owning actor."), *GetNameSafe(Animation));
		return;
	}

	// Retrieve the equipment component from the mesh's owning actor, which should be a pawn.
	APawn* OwnerAsPawn = Cast<APawn>(MeshComp->GetOwner());
	UEquipmentComponent* EquipmentComp = (OwnerAsPawn ? UEquipmentComponent::FindEquipmentComponent(OwnerAsPawn) : nullptr);

	if (!IsValid(EquipmentComp))
	{
		EQUIPMENT_LOG(Warning, TEXT("Attempted to end a \"Hide Equipment\" notify state in animation [%s], but its owning actor [%s] does not have an equipment component."), *GetNameSafe(Animation), *GetNameSafe(MeshComp->GetOwner()));
		return;
	}

	// Iterate through each equipment actor for the current equipment and unhide it.
	if (const UEquipmentInstance* EquipmentInstance = EquipmentComp->GetEquipment())
	{
		for (const AEquipmentActor* EquipmentActor : EquipmentInstance->GetSpawnedActors())
		{
			if (IsValid(EquipmentActor))
			{
				if (USceneComponent* Root = EquipmentActor->GetRootComponent())
				{
					/* Only hide equipment attached to the owning mesh, so third-person animations don't hide first-person 
					 * equipment or vice versa. */
					if (MeshComp->GetAttachChildren().Contains(Root))
					{
						/* Use HiddenInGame instead of Visibility to maintain perspectives. Equipment actors' perspective-based
						 * visibility (e.g. hiding first-person actors when in third-person) is managed by Visibility. */
						Root->SetHiddenInGame(false, true);
					}
				}
			}
		}
	}
}