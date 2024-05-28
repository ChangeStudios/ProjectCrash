// Copyright Samuel Reitich 2024.


#include "Animation/AnimNotifies/AnimNotify_PlayEquipmentMontage.h"


UAnimNotify_PlayEquipmentMontage::UAnimNotify_PlayEquipmentMontage()
{
}

FString UAnimNotify_PlayEquipmentMontage::GetNotifyName_Implementation() const
{
	return Super::GetNotifyName_Implementation();
}

void UAnimNotify_PlayEquipmentMontage::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
}
