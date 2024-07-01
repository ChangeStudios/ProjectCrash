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

	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;



	// Attributes.

// Attribute accessors.
public:

	ATTRIBUTE_ACCESSORS(UMovementAttributeSet, MaxWalkSpeed);
	ATTRIBUTE_ACCESSORS(UMovementAttributeSet, JumpVelocity);
	ATTRIBUTE_ACCESSORS(UMovementAttributeSet, JumpCount);

// Attribute properties.
private:

	/** The avatar's maximum walking speed. */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxWalkSpeed, Category = "Ability|Attribute|Movement", Meta = (HideFromModifiers, AllowPrivateAccess = true))
	FGameplayAttributeData MaxWalkSpeed;

	/** The velocity with which the avatar is launched when jumping. */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_JumpVelocity, Category = "Ability|Attribute|Movement", Meta = (HideFromModifiers, AllowPrivateAccess = true))
	FGameplayAttributeData JumpVelocity;

	/** The maximum number of jumps the avatar is allowed to make before landing. */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_JumpCount, Category = "Ability|Attribute|Movement", Meta = (HideFromModifiers, AllowPrivateAccess = true))
	FGameplayAttributeData JumpCount;



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
};
