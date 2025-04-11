// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AttributeSets/CrashAttributeSet.h"
#include "MovementAttributeSet.generated.h"

/**
 * Attribute set that determines characters' movement properties. Only used if the ASC's avatar has a movement
 * component of type CrashCharacterMovementComponent. 
 */
UCLASS(BlueprintType)
class PROJECTCRASH_API UMovementAttributeSet : public UCrashAttributeSet
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	UMovementAttributeSet();



	// Attribute changes.

protected:

	/** Called before an attribute's base value is changed. Clamps the new base value. */
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;

	/** Called before an attribute is modified. Clamps the new value. */
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	/** Clamps the given attribute between its minimum and maximum values, depending on the attribute. */
	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;



	// Attributes.

// Attribute accessors.
public:

	ATTRIBUTE_ACCESSORS(UMovementAttributeSet, MaxWalkSpeed);
	ATTRIBUTE_ACCESSORS(UMovementAttributeSet, JumpVelocity);
	ATTRIBUTE_ACCESSORS(UMovementAttributeSet, JumpCount);
	ATTRIBUTE_ACCESSORS(UMovementAttributeSet, GravityScale);
	ATTRIBUTE_ACCESSORS(UMovementAttributeSet, AirControl);

// Attribute properties.
private:

	/** The avatar's maximum walking speed. Should only be modified by scaling. */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxWalkSpeed, Category = "Ability|Attribute|Movement", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxWalkSpeed;

	/** The velocity with which the avatar is launched when jumping. Should only be modified by scaling. */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_JumpVelocity, Category = "Ability|Attribute|Movement", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData JumpVelocity;

	/** The maximum number of jumps the avatar is allowed to make before landing. */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_JumpCount, Category = "Ability|Attribute|Movement", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData JumpCount;

	/** Scalar applied to the global gravity constant to determine gravity magnitude used for this character. */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_GravityScale, Category = "Ability|Attribute|Movement", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData GravityScale;

	/** The amount of lateral movement control characters have while airborne. */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_AirControl, Category = "Ability|Attribute|Movement", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData AirControl;



	// Attribute delegates. Most variables in these delegates will not be valid on clients.

public:

	/** Delegate broadcast when the MaxWalkSpeed attribute changes. */
	mutable FAttributeChangedSignature MaxWalkSpeedAttributeChangedDelegate;

	/** Delegate broadcast when the JumpVelocity attribute changes. */
	mutable FAttributeChangedSignature JumpVelocityAttributeChangedDelegate;

	/** Delegate broadcast when the JumpCount attribute changes. */
	mutable FAttributeChangedSignature JumpCountAttributeChangedDelegate;

	/** Delegate broadcast when the GravityScale attribute changes. */
	mutable FAttributeChangedSignature GravityScaleAttributeChangedDelegate;

	/** Delegate broadcast when the AirControl attribute changes. */
	mutable FAttributeChangedSignature AirControlAttributeChangedDelegate;



	// OnRep functions for attribute changes.

protected:

	/** OnRep for MaxWalkSpeed. */
	UFUNCTION()
	void OnRep_MaxWalkSpeed(const FGameplayAttributeData& OldValue);

	/** OnRep for JumpVelocity. */
	UFUNCTION()
	void OnRep_JumpVelocity(const FGameplayAttributeData& OldValue);

	/** OnRep for JumpCount. */
	UFUNCTION()
	void OnRep_JumpCount(const FGameplayAttributeData& OldValue);

	/** OnRep for GravityScale. */
	UFUNCTION()
	void OnRep_GravityScale(const FGameplayAttributeData& OldValue);

	/** OnRep for AirControl. */
	UFUNCTION()
	void OnRep_AirControl(const FGameplayAttributeData& OldValue);
};
