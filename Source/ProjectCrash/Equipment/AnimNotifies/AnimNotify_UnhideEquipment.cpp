// Copyright Samuel Reitich. All rights reserved.


#include "Equipment/AnimNotifies/AnimNotify_UnhideEquipment.h"

#include "Equipment/EquipmentActor.h"
#include "Equipment/EquipmentComponent.h"
#include "Equipment/EquipmentInstance.h"
#include "GameFramework/CrashLogging.h"

UAnimNotify_UnhideEquipment::UAnimNotify_UnhideEquipment()
{
#if WITH_EDITORONLY_DATA
	// Notify's default color in the editor.
	NotifyColor = FColor(75, 225, 75);
#endif

	// We won't have valid equipment in the editor, so there's no point in firing this.
	bShouldFireInEditor = false;
}

void UAnimNotify_UnhideEquipment::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	// Make sure mesh and owning actor (the pawn with equipment) are valid.
	if (!IsValid(MeshComp) || !(IsValid(MeshComp->GetOwner())))
	{
		EQUIPMENT_LOG(Warning, TEXT("Attempted to trigger a \"Unhide Equipment\" notify in animation [%s], but it does not have a valid owning actor."), *GetNameSafe(Animation));
		return;
	}

	// Retrieve the equipment component from the mesh's owning actor, which should be a pawn.
	APawn* OwnerAsPawn = Cast<APawn>(MeshComp->GetOwner());
	UEquipmentComponent* EquipmentComp = (OwnerAsPawn ? UEquipmentComponent::FindEquipmentComponent(OwnerAsPawn) : nullptr);

	if (!EquipmentComp)
	{
		EQUIPMENT_LOG(Warning, TEXT("Attempted to trigger a \"Unhide Equipment\" notify in animation [%s], but its owning actor [%s] does not have an equipment component."), *GetNameSafe(Animation), *GetNameSafe(MeshComp->GetOwner()));
		return;
	}

	// Iterate through each equipment actor for the current equipment and unhide it.
	if (EquipmentComp->GetEquipment())
	{
		for (AEquipmentActor* EquipmentActor : EquipmentComp->GetEquipment()->GetSpawnedActors())
		{
			if (USceneComponent* Root = EquipmentActor->GetRootComponent())
			{
				/* Use HiddenInGame instead of Visibility to maintain perspectives. Equipment actors' perspective-based
				 * visibility (e.g. hiding first-person actors when in third-person) is managed by Visibility. */
				Root->SetHiddenInGame(false, true);
			}
		}
	}
}
