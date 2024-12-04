// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_TriggerGameplayEvent.generated.h"

/**
 * An animation notify that triggers a gameplay event with a given gameplay event tag.
 *
 * The gameplay event is sent to the owner of the mesh on which this notify is triggered. This is often paired with
 * an animation played by PlayDualMontageAndWaitForEvent.
 *
 * When used in ability animations, this notify should preferably be placed in the third-person animation, rather than
 * the first-person animation. The third-person mesh is guaranteed to always be in sync with the network, unlike the
 * first-person mesh.
 */
UCLASS(Const, HideCategories = Object, CollapseCategories, DisplayName = "Trigger Gameplay Event")
class PROJECTCRASH_API UAnimNotify_TriggerGameplayEvent : public UAnimNotify
{
	GENERATED_BODY()

	// Animation notify.

public:

	/** Default constructor. */
	UAnimNotify_TriggerGameplayEvent();

	/** Displays this notify's gameplay event tag as its name, if the tag is valid. */
	virtual FString GetNotifyName_Implementation() const override;

	/** Called when this notify is triggered. Sends a gameplay event to the owner of the mesh that triggered this
	 * notify using the event tag specified by this notify. */
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;



	// User-exposed parameters.

public:

	/** The gameplay tag used to trigger the gameplay event when this notify is triggered. The event is sent to the
	 * owner of the mesh that triggered this notify. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify", DisplayName = "Gameplay Event Tag")
	FGameplayTag EventTag;
};
