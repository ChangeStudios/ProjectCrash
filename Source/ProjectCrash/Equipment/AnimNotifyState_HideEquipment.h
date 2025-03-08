// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_HideEquipment.generated.h"

/**
 * Locally hides this character's equipment meshes while active. Note that this only affects equipment attached to the
 * mesh on which this notify plays.
 */
UCLASS(Const, HideCategories = Object, CollapseCategories, DisplayName = "Hide Equipment")
class PROJECTCRASH_API UAnimNotifyState_HideEquipment : public UAnimNotifyState
{
	GENERATED_BODY()

	// Animation notify.

public:

	/** Default constructor. */
	UAnimNotifyState_HideEquipment();

	/** Overrides notify name. */
	virtual FString GetNotifyName_Implementation() const override { return "Hide Equipment"; }

	/** Hides equipment owned by this character. */
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

	/** Unhides equipment owned by this character. */
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;



	// Notify parameters.

public:

	/** Hide specific equipment. If unset, all current equipment will be hidden. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify", Meta = (Categories = "Equipment"))
	FGameplayTagContainer FilterEquipment;
};
