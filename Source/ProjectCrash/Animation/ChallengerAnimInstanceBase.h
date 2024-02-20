// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ChallengerAnimInstanceBase.generated.h"

// Animation log channel shorthand.
#define ANIMATION_LOG(Verbosity, Format, ...) \
{ \
	UE_LOG(LogAnimation, Verbosity, Format, ##__VA_ARGS__); \
}

class AChallengerBase;
class UCharacterAnimData;
class UCrashAbilitySystemComponent;
class UEquipmentAnimationData;

/**
 * The base animation instance for all challenger character animation instances. Provides utilities that both first-
 * person and third-person animation instances utilize.
 */
UCLASS()
class PROJECTCRASH_API UChallengerAnimInstanceBase : public UAnimInstance
{
	GENERATED_BODY()

	// Initialization.

public:

	/** Caches static information about this animation instance's owning character. */
	virtual void NativeBeginPlay() override;



	// Utils.

public:

	/** Thread-safe function for checking if an ASC has a given tag. */
	UFUNCTION(BlueprintCallable, Category = "Characters|Challenger|Animation", Meta = (BlueprintThreadSafe))
	bool ThreadSafeHasTagExact(UAbilitySystemComponent* ASC, FGameplayTag TagToSearch) const;

protected:

	/** This animation instance's owning pawn, cached for convenience. */
	UPROPERTY(BlueprintReadOnly, Category = "Characters|Challenger|Animation")
	TObjectPtr<AChallengerBase> OwningChallenger;

	/** This animation instance's owning pawn's ASC, cached for convenience. */
	UPROPERTY(BlueprintReadOnly, Category = "Characters|Challenger|Animation")
	TObjectPtr<UCrashAbilitySystemComponent> OwningASC;

	/** Caches this animation's owning pawn's ASC after it is initialized. */
	UFUNCTION()
	void OnASCInitialized(UCrashAbilitySystemComponent* CrashASC);



	// Animation data.

public:

	/** Updates this animation instance's current animation data. */
	UFUNCTION(BlueprintCallable, Category = "Animation|Data")
	void UpdateAnimData(UCharacterAnimData* NewAnimData);

protected:

	/** The animation data currently being used by this animation instance. */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Data")
	TObjectPtr<UCharacterAnimData> CurrentAnimationData;
};
