// Copyright Samuel Reitich. All rights reserved.


#include "Animation/AnimNotifies/AnimNotify_PlayNiagaraEffectWithPerspective.h"

#include "Particles/ParticleSystemComponent.h"


void UAnimNotify_PlayNiagaraEffectWithPerspective::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	// Update the spawned effect's visibility to match that of its parent mesh.
	if (SpawnedEffect)
	{
		SpawnedEffect->SetVisibility(MeshComp->IsVisible(), true);
	}
}
