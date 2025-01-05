// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_TriggerAbilityTargeting.generated.h"

class UAbilitySystemComponent;
/**
 * Triggers an "Ability.TargetingStart" gameplay event when the notify starts and an "Ability.TargetingEnd" gameplay
 * event when the notify ends. Used to determine the targeting duration of a specified ability, such as when a melee
 * attack can hit targets during its animation.
 *
 * Triggered gameplay events use an ability identifier to prevent accidentally triggering a different ability's
 * targeting. This allows us to avoid having to create two custom event tags for every ability that requires event-based
 * targeting.
 */
UCLASS(Const, HideCategories = Object, CollapseCategories, DisplayName = "Trigger Ability Targeting", Meta = (
	ToolTip = "Triggers a specified ability's targeting by triggering \"Ability.TargetingStart\" and \"Ability.TargetingEnd\" gameplay events at the start and end of the notify. Used to determine the targeting duration of a ability, such as when a melee attack can hit targets during its animation."))
class PROJECTCRASH_API UAnimNotifyState_TriggerAbilityTargeting : public UAnimNotifyState
{
	GENERATED_BODY()

	// Anim notify.

public:

	/** Default constructor. */
	UAnimNotifyState_TriggerAbilityTargeting();

	/** Displays the target ability in the notify name. */
	virtual FString GetNotifyName_Implementation() const override;

	/** Fires the "Ability.TargetingStart" gameplay event, passing the specified ability's identifier in the payload. */
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

	/** Fires the "Ability.TargetingEnd" gameplay event, passing the specified ability's identifier in the payload. */
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;



	// Editor parameters.

protected:

	/** The identifying tag of the ability for which to trigger targeting. Prevents this notify from accidentally
	 * triggering another ability's targeting. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify", Meta = (Categories = "Ability.Identifier"))
	FGameplayTag AbilityIdentifier;
};
