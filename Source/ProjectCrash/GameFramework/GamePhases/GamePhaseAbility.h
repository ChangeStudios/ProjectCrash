// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/CrashGameplayAbilityBase.h"
#include "GamePhaseAbility.generated.h"

/**
 * 
 */
UCLASS(Abstract, HideCategories = ("InputTag", "Ability Activation", "User Interface"))
class PROJECTCRASH_API UGamePhaseAbility : public UCrashGameplayAbilityBase
{
	GENERATED_BODY()

	// Initialization.
	
public:

	UGamePhaseAbility(const FObjectInitializer& ObjectInitializer);



	// Ability.

protected:

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;



	// Game phase.

public:

	const FGameplayTag& GetGamePhaseTag() const { return GamePhaseTag; }

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Phase")
	FGameplayTag GamePhaseTag;



	// Validation.

public:

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
};
