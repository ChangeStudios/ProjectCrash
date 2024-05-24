// Copyright Samuel Reitich 2024.


#include "Animation/AnimNotifies/AnimNotify_TriggerEquipmentEffect.h"

#include "Equipment/EquipmentComponent.h"


UAnimNotify_TriggerEquipmentEffect::UAnimNotify_TriggerEquipmentEffect()
	: Super()
{
	// Set this notify's default color in the editor.
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(127, 127, 0, 255);
#endif
}

FString UAnimNotify_TriggerEquipmentEffect::GetNotifyName_Implementation() const
{
	// Return the name of the effect tag used by this notify, if it has been set.
	if (EffectTag.IsValid())
	{
		return EffectTag.ToString();
	}

	// Return the notify's default name if an effect tag has not been set.
	return Super::GetNotifyName_Implementation();
}

void UAnimNotify_TriggerEquipmentEffect::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	// Ensure we have valid references to the executing skeletal mesh component and its owning actor.
	if (!IsValid(MeshComp) || !IsValid(MeshComp->GetOwner()))
	{
		UE_LOG(LogAnimation, Warning, TEXT("UAnimNotify_TriggerEquipmentEffect: Tried to send an equipment effect in animation [%s], but the skeletal mesh component and/or its owning actor are not valid."), *GetNameSafe(Animation));
		return;
	}

	// Retrieve the owner's equipment component.
	UEquipmentComponent* EquipmentComponent = UEquipmentComponent::FindEquipmentComponent(MeshComp->GetOwner());
	if (!IsValid(EquipmentComponent))
	{
		UE_LOG(LogAnimation, Warning, TEXT("UAnimNotify_TriggerEquipmentEffect: Tried to send a equipment effect in animation [%s], but the owning actor, [%s], does not have a valid equipment component."), *GetNameSafe(Animation), *MeshComp->GetOwner()->GetName());
		return;
	}

	// Ensure the specified gameplay tag for the effect is valid.
	if (!EffectTag.IsValid())
	{
		UE_LOG(LogAnimation, Warning, TEXT("UAnimNotify_TriggerEquipmentEffect: Tried to send a equipment effect in animation [%s] for [%s], but the effect tag is not set or is not valid."), *GetNameSafe(Animation), *MeshComp->GetOwner()->GetName());
		return;
	}

	// Send the effect to the equipment component.
	if (!bEnd)
	{
	}
	// End the effect through the equipment component.
	else
	{
	}

	Super::Notify(MeshComp, Animation, EventReference);
}
