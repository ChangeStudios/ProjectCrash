// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "AbilityDataAsset.h"
#include "AbilityData_Challenger_Knight.generated.h"

/**
 * Static data for the Knight challenger's abilities.
 */
UCLASS()
class PROJECTCRASH_API UAbilityData_Challenger_Knight : public UAbilityDataAsset
{
	GENERATED_BODY()

public:

	// Sword.

	/** Damage dealt by the Sword ability. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sword", DisplayName = "Sword Damage")
	float Sword_Damage;

	/** Radius of the sphere-trace performed from the player camera during the attack to check for Sword targets. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sword", DisplayName = "Sword Radius", Meta = (Units = "cm"))
	float Sword_Radius;

	/** Range of the sphere-trace performed from the player camera during the attack to check for Sword targets. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sword", DisplayName = "Sword Range", Meta = (Units = "cm"))
	float Sword_Range;



	// Parry.

	/** Damage dealt by the Parry ability. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Parry", DisplayName = "Parry Damage")
	float Parry_Damage;

	/** Force with which the character dashes forward when using the Parry ability. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Parry", DisplayName = "Parry Dash Force", Meta = (Units = "CentimetersPerSecond"))
	float Parry_DashForce;

	/** Duration for which the character dashes forward when using the Parry ability. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Parry", DisplayName = "Parry Duration", Meta = (Units = "s"))
	float Parry_Duration;

	/** Force with which targets are knocked back when hit by the Parry ability. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Parry", DisplayName = "Parry Knockback Force", Meta = (Units = "CentimetersPerSecond"))
	float Parry_KnockbackForce;

	/** Half-height of the capsule traced around the player to check for Parry targets. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Parry", DisplayName = "Parry Capsule Half-Height", Meta = (Units = "cm"))
	float Parry_TargetCapsule_HalfHeight;

	/** Radius of the capsule traced around the player to check for Parry targets. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Parry", DisplayName = "Parry Capsule Radius", Meta = (Units = "cm"))
	float Parry_TargetCapsule_Radius;



	// Joust.

	/** Damage dealt by the Joust ability. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Joust", DisplayName = "Joust Damage")
	float Joust_Damage;

	/** Force with which the character dashes forward when using the Joust ability. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Joust", DisplayName = "Joust Dash Force", Meta = (Units = "CentimetersPerSecond"))
	float Joust_DashForce;

	/** Duration for which the character dashes forward when using the Joust ability. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Joust", DisplayName = "Joust Duration", Meta = (Units = "s"))
	float Joust_Duration;

	/** Force with which targets are knocked back when hit by the Joust ability. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Joust", DisplayName = "Joust Knockback Force", Meta = (Units = "CentimetersPerSecond"))
	float Joust_KnockbackForce;

	/** Half-height of the capsule traced around the player to check for Joust targets. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Joust", DisplayName = "Joust Capsule Half-Height", Meta = (Units = "cm"))
	float Joust_TargetCapsule_HalfHeight;

	/** Radius of the capsule traced around the player to check for Joust targets. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Joust", DisplayName = "Joust Capsule Radius", Meta = (Units = "cm"))
	float Joust_TargetCapsule_Radius;



	// War Hammer.

	/** Damage dealt by the War Hammer ability. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "War Hammer", DisplayName = "War Hammer Damage")
	float WarHammer_Damage;

	/** Duration for which the character falls to the ground when activating the War Hammer ability in the air.
	 * Downward velocity is scaled to always match this. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "War Hammer", DisplayName = "War Hammer Fall Duration", Meta = (Units = "s"))
	float WarHammer_FallDuration;

	/** Force with which targets are knocked away when hit by the War Hammer ability. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "War Hammer", DisplayName = "War Hammer Force", Meta = (Units = "CentimetersPerSecond"))
	float WarHammer_Force;

	/** Radius around the character to check for War Hammer targets. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "War Hammer", DisplayName = "War Hammer Radius", Meta = (Units = "cm"))
	float WarHammer_Radius;

	
};
