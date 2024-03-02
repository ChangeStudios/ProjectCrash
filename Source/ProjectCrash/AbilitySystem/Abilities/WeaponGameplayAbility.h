// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/CrashGameplayAbilityBase.h"
#include "WeaponGameplayAbility.generated.h"

/**
 * A gameplay ability that represents an attack with a weapon. Provides functionality for limited ammo and reloading.
 */
UCLASS(Abstract)
class PROJECTCRASH_API UWeaponGameplayAbility : public UCrashGameplayAbilityBase
{
	GENERATED_BODY()

public:

	/** The gameplay tag that represents this weapon's current ammunition. The number of these tags owned by the player
	 * represents this weapon's ammo count. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon")
	FGameplayTag CurrentAmmoTag;

	/** The gameplay tag that represents this weapon's maximum ammunition. The number of these tags owned by the player
	 * represents the weapon's maximum ammo count. This usually does not change during runtime. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon")
	FGameplayTag MaximumAmmoTag;
};
