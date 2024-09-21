// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_PlayEquipmentAnimation.generated.h"

/**
 * Plays an animation on the owning pawn's current equipment actors using a tag to indicate which animation to play.
 * Each equipment actor will check its EquipmentAnimations map for an animation matching the given tag and play that
 * animation.
 * 
 * This method is designed to support multiple active equipment actors. If one animation was passed to each equipment
 * actor, then every equipment actor (e.g. a sword AND a shield actor) would be playing the same animation, which is
 * likely not desirable. This method allows each equipment actor to specify its own set of animations to play in
 * response to each animation request.
 */
UCLASS(Const, HideCategories = Object, DisplayName = "Play Equipment Animation")
class PROJECTCRASH_API UAnimNotify_PlayEquipmentAnimation : public UAnimNotify
{
	GENERATED_BODY()

	// Anim notify.

public:

	/** Default constructor. */
	UAnimNotify_PlayEquipmentAnimation();

	/** Uses the equipment animation tag as the notify's name. */
	virtual FString GetNotifyName_Implementation() const override;

	/** Routes a specified animation tag to the owning pawn's current equipment actors. */
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;



	// Editor parameters.

protected:

	/** Tag used to indicate which animation to play. Any active equipment actor with this tag in their
	 * EquipmentAnimations map will play that animation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify", Meta = (Categories = "SkinProperty"))
	FGameplayTag AnimationTag;
};
