// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_UnhideEquipment.generated.h"

/**
 * Locally unhides this character's equipment actors when triggered. Note that this affects both first- and third-person
 * equipment actors.
 */
UCLASS(Const, HideCategories = Object, CollapseCategories, DisplayName = "Unhide Equipment")
class PROJECTCRASH_API UAnimNotify_UnhideEquipment : public UAnimNotify
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	UAnimNotify_UnhideEquipment();

	/** Unhides equipment owned by this character. */
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
