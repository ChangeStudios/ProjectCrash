// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "AbilitySystem/Abilities/CrashGameplayAbilityBase.h"
#include "GA_Jump.generated.h"

UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_Ability_Identifier_Generic_Jump);

/**
 * The default character jump ability. This uses the built-in character jump method, meaning its properties (e.g.
 * jump height) can be altered in the owning character's class and movement component, including the implementation of
 * double-jump.
 */
UCLASS()
class PROJECTCRASH_API UGA_Jump : public UCrashGameplayAbilityBase
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	UGA_Jump(const FObjectInitializer& ObjectInitializer);

	/** Ends this ability when its input is released. */
	virtual void OnInputReleased() override;

protected:

	/** Ensures character can jump. Checks conditions like available space, if the character is airborne, if the
	 * character has remaining jumps, etc. */
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;

	/** Attempts to trigger the built-in character jump method. */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/** Forces the StopJumping method in case it wasn't handled automatically. */
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
};
