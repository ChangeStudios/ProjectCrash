// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_PlayEquipmentMontage.generated.h"

/**
 * An animation notify that plays an animation montage on the equipment set currently equipped by the owner of this
 * animation. Any piece with a matching gameplay tag in their Animations map will play the montage mapped to that tag.
 *
 * This is used to play different equipment animations (and any effects nested within them) depending on the skin,
 * while still triggering animations in a skin-agnostic manner.
 */
UCLASS(Const, HideCategories = Object, CollapseCategories, DisplayName = "Play Equipment Montage")
class PROJECTCRASH_API UAnimNotify_PlayEquipmentMontage : public UAnimNotify
{
	GENERATED_BODY()
	
	// Animation notify.

public:

	/** Default constructor. */
	UAnimNotify_PlayEquipmentMontage();

	/** Displays this notify's equipment animation tag as its name, if the tag has been set. */
	virtual FString GetNotifyName_Implementation() const override;

	/** Called when this notify is triggered. Plays an animation montage on any active equipment pieces that have the
	 * given animation tag defined. */
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;



	// User-exposed parameters.

public:

	/** The tag representing which montages should be played when this notify is triggered. Any equipment pieces with
	 * this tag defined will play the corresponding montage defined in the equipment piece's animation map. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify", DisplayName = "Equipment Animation", Meta = (Categories = "EquipmentAnimation"))
	FGameplayTag EquipmentAnimTag;
};
