// Copyright Samuel Reitich. All rights reserved.


#include "Equipment/AnimNotifyState_TimedEquipmentNiagaraEffect.h"

#include "Particles/ParticleSystemComponent.h"

void UAnimNotifyState_TimedEquipmentNiagaraEffect::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	// Update the spawned effect's visibility to match that of its parent mesh.
	if (UFXSystemComponent* FXComponent = GetSpawnedEffect(MeshComp))
	{
		FXComponent->SetVisibility(MeshComp->IsVisible(), true);
	}
}
