// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CrashCharacterMovementComponent.generated.h"

class UMovementAttributeSet;

/** Delegate signature for broadcasting when a character lands on the ground via its movement component (as opposed to
 * its character). */
DECLARE_MULTICAST_DELEGATE_OneParam(FMoveCompLandedSignature, const FHitResult& /* Hit */);

/**
 * Default character movement component for this project. Integrates with its owner's MovementAttributeSet, if one
 * exists.
 *
 * The following values are controlled by attributes (thus, the values set in archetypes are ignored if the owning
 * character has a movement attribute set):
 *  - MaxWalkSpeed
 *  - JumpZVelocity
 *  - JumpCount (from ACharacter)
 *  - GravityScale
 */
UCLASS(BlueprintType, ClassGroup = "Movement", Meta = (BlueprintSpawnableComponent))
class PROJECTCRASH_API UCrashCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	UCrashCharacterMovementComponent(const FObjectInitializer& ObjectInitializer);



	// Initialization.

public:

	/** Must be called by the owning actor to initialize this component with an ASC and updating movement properties
	 * with attributes. The given ASC must have a MovementAttributeSet. */
	UFUNCTION(BlueprintCallable, Category = "Ability|Attribute|Movement")
	void InitializeWithAbilitySystem(UCrashAbilitySystemComponent* InASC);

	/** Uninitializes this component from its current ASC. Should be called by the owning actor to stop using movement
	 * attributes. Called automatically when this component is unregistered. */
	UFUNCTION(BlueprintCallable, Category = "Ability|Attribute|Movement")
	void UninitializeFromAbilitySystem();

protected:

	/** Automatically uninitializes this component from the ability system when it's unregistered. */
	virtual void OnUnregister() override;



	// Movement.

public:

	/** Fired when this character lands on the ground. */
	FMoveCompLandedSignature LandedDelegate;

protected:

	/** Fires LandedDelegate when this character lands. */
	virtual void ProcessLanded(const FHitResult& Hit, float remainingTime, int32 Iterations) override;



	/* Attribute changes callbacks, responsible for updating the movement component's properties using corresponding
	 * attributes. */

protected:

	/** Updates this movement component's MaxWalkSpeed property. */
	UFUNCTION()
	void OnMaxWalkSpeedChanged(AActor* EffectInstigator, const FGameplayEffectSpec& EffectSpec, float OldValue, float NewValue);

	/** Updates this movement component's JumpZVelocity property. */
	UFUNCTION()
	void OnJumpVelocityChanged(AActor* EffectInstigator, const FGameplayEffectSpec& EffectSpec, float OldValue, float NewValue);

	/** Updates this component's owning character's JumpCount property. */
	UFUNCTION()
	void OnJumpCountChanged(AActor* EffectInstigator, const FGameplayEffectSpec& EffectSpec, float OldValue, float NewValue);

	/** Updates this movement component's GravityScale property. */
	UFUNCTION()
	void OnGravityScaleChanged(AActor* EffectInstigator, const FGameplayEffectSpec& EffectSpec, float OldValue, float NewValue);



	// Internals.

protected:

	/** Ability system to which this component is currently bound. */
	UPROPERTY()
	TObjectPtr<UCrashAbilitySystemComponent> AbilitySystemComponent;

	/** Movement attribute set to which this component is currently bound. Owned by AbilitySystemComponent. */
	UPROPERTY()
	TObjectPtr<const UMovementAttributeSet> MovementSet;



	// Utils.

public:

	/** Retrieves the given actor's CrashCharacterMovementComponent, if it has one. Returns null otherwise. */
	UFUNCTION(BlueprintPure, Category = "Ability|Attribute|Movement")
	static UCrashCharacterMovementComponent* FindCrashMovementComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<UCrashCharacterMovementComponent>() : nullptr); }

	/** Gets the distance from the bottom of this character's capsule to the ground beneath them. */
	float GetGroundDistance() const;
};
