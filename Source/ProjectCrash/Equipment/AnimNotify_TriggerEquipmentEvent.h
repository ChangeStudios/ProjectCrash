// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_TriggerEquipmentEvent.generated.h"

/**
 * Sends an equipment event to this animation's owning actor. The actor needs to implement IEquipmentEventInterface to
 * process the event.
 */
UCLASS(Const, HideCategories = Object, CollapseCategories, DisplayName = "Trigger Equipment Event")
class PROJECTCRASH_API UAnimNotify_TriggerEquipmentEvent : public UAnimNotify
{
	GENERATED_BODY()
	
	// Animation notify.

public:

	/** Default constructor. */
	UAnimNotify_TriggerEquipmentEvent();

	/** Uses this notify's equipment event tag as its name. */
	virtual FString GetNotifyName_Implementation() const override;

	/** Routes the equipment event to the owner of this animation's mesh, if it implements IEquipmentEventInterface. */
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;



	// User-exposed parameters.

public:

	/** The gameplay tag used to trigger the gameplay event when this notify is triggered. The event is sent to the
	 * owner of the mesh that triggered this notify. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify", DisplayName = "Equipment Event Tag")
	FGameplayTag EventTag;
};
