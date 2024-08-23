// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AnimNotifyState_TimedNiagaraEffect.h"
#include "AnimNotifyState_TimedEquipmentNiagaraEffect.generated.h"

/**
 * Timed niagara effect that supports perspective-based visibility. The effect will only be visible if the mesh on which
 * it's being played is visible.
 */
UCLASS(Blueprintable, Meta = (DisplayName = "Timed Equipment Niagara Effect"), MinimalAPI)
class UAnimNotifyState_TimedEquipmentNiagaraEffect : public UAnimNotifyState_TimedNiagaraEffect
{
	GENERATED_BODY()

protected:

	/** After spawning the effect, updates its visibility to match that of its parent mesh component. All other
	 * visibility updates will be driven by the view target's perspective changes. */
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
};
