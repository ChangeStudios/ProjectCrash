// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/CrashGameplayAbilityBase.h"
#include "CrashGameplayAbility_Death.generated.h"

/**
 * The base class for "Death" abilities. This is triggered for the ASC of dying actors with the
 * "GameplayEvent.Ability.Death" event. This is activated automatically by the HealthAttributeSet and HealthComponent.
 *
 * This should be subclassed to handle user-facing death logic, such as changing camera modes or playing VFX.
 */
UCLASS(Abstract)
class PROJECTCRASH_API UCrashGameplayAbility_Death : public UCrashGameplayAbilityBase
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	UCrashGameplayAbility_Death(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:

	/** Starts the avatar actor's death sequence through their health component. Also cancels other active abilities. */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/** Finishes the avatar actor's death sequence through their health component. */
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:

	/** The dying avatar actor. Cached in case the player respawns with a new avatar before this ability ends. */
	UPROPERTY()
	TObjectPtr<AActor> DyingActor;
};
