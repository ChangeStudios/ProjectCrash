// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "Animation/AnimNotifies/AnimNotify.h"
#include "EquipmentActor.h"
#include "GameplayTagContainer.h"
#include "AnimNotify_PlayEquipmentAnimation.generated.h"

/**
 * Plays a specified animation montage on a specified equipment mesh.
 */
UCLASS(Const, HideCategories = Object, DisplayName = "Play Equipment Animation")
class PROJECTCRASH_API UAnimNotify_PlayEquipmentAnimation : public UAnimNotify
{
	GENERATED_BODY()

	// Anim notify.

public:

	/** Default constructor. */
	UAnimNotify_PlayEquipmentAnimation();

	/** Uses the montage as the notify's name. */
	virtual FString GetNotifyName_Implementation() const override;

	/** Plays a specified animation on equipment meshes with the matching tag. */
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;



	// Editor parameters.

protected:

	/** The animation to play on the equipment mesh. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify")
	UAnimMontage* EquipmentAnimation;

	/** The identifying tag of the equipment mesh this animation should be played on. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify", Meta = (Categories = "Equipment"))
	FGameplayTag Equipment;

	/** Which perspective to play the animation in. You can use the same equipment animation for all perspectives by
	 * using this notify in both the first- and third-person character animations. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify", DisplayName = "Perspective to Play In")
	EEquipmentPerspective EquipmentPerspective;
};
