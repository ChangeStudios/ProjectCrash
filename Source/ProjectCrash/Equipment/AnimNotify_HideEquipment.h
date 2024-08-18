// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_HideEquipment.generated.h"

/**
 * Locally hides this character's equipment actors when triggered. Note that this affects both first- and third-person
 * equipment actors.
 */
UCLASS(Const, HideCategories = Object, CollapseCategories, DisplayName = "Hide Equipment")
class PROJECTCRASH_API UAnimNotify_HideEquipment : public UAnimNotify
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	UAnimNotify_HideEquipment();

	/** Hides equipment owned by this character. */
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
