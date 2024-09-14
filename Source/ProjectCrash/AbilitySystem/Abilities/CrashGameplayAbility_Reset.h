// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/CrashGameplayAbilityBase.h"
#include "CrashGameplayAbility_Reset.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTCRASH_API UCrashGameplayAbility_Reset : public UCrashGameplayAbilityBase
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	UCrashGameplayAbility_Reset(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:

	/** Resets the activating player. */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};



USTRUCT(BlueprintType)
struct FCrashPlayerResetMessage
{
	GENERATED_BODY()

	/** The owner of the ASC that performed the reset. Usually the player state of the character that was reset. */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> OwningActor = nullptr;

	/** Controller of the character that was reset. */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AController> Controller = nullptr;
};
