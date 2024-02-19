// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_TriggerGameplayEvent.generated.h"

/**
 * An animation notify that triggers a gameplay event with a given gameplay event tag.
 *
 * The gameplay event is sent to the owner of the mesh on which this notify is triggered. This is most efficiently used
 * in abilities with a PlayMontageAndWait followed by a WaitGameplayEventToActor node.
 */
UCLASS(Const, HideCategories = Object, CollapseCategories, Meta = (DisplayName = "Trigger Gameplay Event"))
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify", Meta = (DisplayName = "Gameplay Event Tag"))
	FGameplayTag EventTag;
};
