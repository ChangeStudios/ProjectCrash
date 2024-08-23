// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AnimNotify_PlayNiagaraEffect.h"
#include "AnimNotify_PlayNiagaraEffectWithPerspective.generated.h"

/**
 * Niagara particle effect that supports perspective-based visibility. The effect will only be visible if the mesh on
 * which it's being played is visible.
 */
UCLASS(Blueprintable, Meta = (DisplayName = "Play Niagara Particle Effect With Perspective"), MinimalAPI)
class UAnimNotify_PlayNiagaraEffectWithPerspective : public UAnimNotify_PlayNiagaraEffect
{
	GENERATED_BODY()

protected:

	/** After spawning the niagara effect, updates its visibility to match that of its parent mesh component. All other
	 * visibility updates will be driven by the view target's perspective changes. */
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
