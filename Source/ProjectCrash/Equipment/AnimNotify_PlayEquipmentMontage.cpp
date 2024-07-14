// Copyright Samuel Reitich. All rights reserved.


#include "AnimNotify_PlayEquipmentMontage.h"

#include "Equipment/EquipmentComponent.h"
#include "GameFramework/CrashLogging.h"


UAnimNotify_PlayEquipmentMontage::UAnimNotify_PlayEquipmentMontage()
{
	// Set this notify's default color in the editor.
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(0, 128, 255, 255);
#endif

	// We won't have valid equipment in the editor, so there's not point in firing this notify.
	bShouldFireInEditor = false;
}

FString UAnimNotify_PlayEquipmentMontage::GetNotifyName_Implementation() const
{
	// Return the name of the equipment anim tag used by this notify, if it has been set.
	if (EquipmentAnimTag.IsValid())
	{
		return EquipmentAnimTag.ToString();
	}

	// Return the notify's default name if an event tag has not been set.
	return Super::GetNotifyName_Implementation();
}

void UAnimNotify_PlayEquipmentMontage::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	// Ensure we have valid references to the executing skeletal mesh component and its owning actor.
	if (!IsValid(MeshComp) || !IsValid(MeshComp->GetOwner()))
	{
		EQUIPMENT_LOG(Warning, TEXT("Attempted to trigger a PlayEquipmentMontage anim notify in animation [%s], but the skeletal mesh component and/or its owning actor are not valid."), *GetNameSafe(Animation));

		return;
	}

	// Make sure we've set our animation tag.
	if (!EquipmentAnimTag.IsValid())
	{
		EQUIPMENT_LOG(Warning, TEXT("Attempted to trigger a PlayEquipmentMontage anim notify on [%s] in animation [%s], but the EquipmentAnimTag has not been set."), *GetNameSafe(MeshComp), *GetNameSafe(Animation));
	}

	// Try to find equipment component on the owner of this notify.
	UEquipmentComponent* EquipmentComp = UEquipmentComponent::FindEquipmentComponent(MeshComp->GetOwner());

	// If an equipment component can't be found, we can't play the equipment montage.
	if (!EquipmentComp)
	{
		EQUIPMENT_LOG(Warning, TEXT("Attempted to trigger a PlayEquipmentMontage anim notify on [%s] in animation [%s], but the mesh's owner, [%s], does not have an equipment component!"), *MeshComp->GetName(), *GetNameSafe(Animation), *MeshComp->GetOwner()->GetName());

		return;
	}

	// Route the animation through the equipment component.
	const bool bSuccess = EquipmentComp->PlayEquipmentMontage(EquipmentAnimTag);

	// Debugging.
	if (!bSuccess)
	{
		EQUIPMENT_LOG(Verbose, TEXT("Successfully triggered a PlayEquipmentMontage anim notify on [%s] in [%s], but did not play any equipment animations. No equipment pieces with animation [%s] were found."), *MeshComp->GetName(), *GetNameSafe(Animation), *EquipmentAnimTag.GetTagName().ToString());
	}

	Super::Notify(MeshComp, Animation, EventReference);
}
