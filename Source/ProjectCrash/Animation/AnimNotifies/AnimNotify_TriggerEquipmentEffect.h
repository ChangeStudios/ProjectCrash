// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_TriggerEquipmentEffect.generated.h"

/**
 * Triggers an equipment effect on the owning character. Equipment effects are routed through the owner's equipment
 * component to each active equipment actor. Can also be used to end ongoing equipment effects.
 *
 * Equipment actors are not replicated, so equipment effects are only triggered locally. But if this animation is
 * replicated, the effect will trigger on any machine that plays this animation.
 */
UCLASS(Const, HideCategories = Object, CollapseCategories, DisplayName = "Trigger Equipment Event")
class PROJECTCRASH_API UAnimNotify_TriggerEquipmentEffect : public UAnimNotify
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	UAnimNotify_TriggerEquipmentEffect();

	/** Displays this notify's equipment effect tag as its name, if the tag is valid. */
	virtual FString GetNotifyName_Implementation() const override;

	/** Routes the specified equipment effect to this animation's owning character's equipment component, if it has
	 * one. */
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;



public:

	/** The effect to trigger. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify", DisplayName = "Equipment Event Tag", Meta = (Categories = "Event.EquipmentEffect"))
	FGameplayTag EffectTag;

	/** Whether the given effect should be started or ended. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify", DisplayName = "End Effect?")
	bool bEnd = false;
};
