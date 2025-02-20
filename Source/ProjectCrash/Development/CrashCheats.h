// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "CrashCheats.generated.h"

class UCrashAbilitySystemComponent;

/**
 * Generic editor cheats.
 */
UCLASS(NotBlueprintable)
class PROJECTCRASH_API UCrashCheats : public UCheatManagerExtension
{
	GENERATED_BODY()

public:

	/** Registers this cheat extension with the cheat manager. */
	UCrashCheats();

	/** Resets the local player, if they have the CrashGameplayAbility_Reset ability. */
	UFUNCTION(Exec)
	void ResetPlayer();

	/** Kills the local player with self-destruct damage. */
	UFUNCTION(Exec)
	void KillPlayer();

	/** Grants the specified percentage (0-1) of ultimate charge. Use Crash.CooldownsDisabled to disable the need for
	 * charge entirely. This is more commonly used to test ultimate charging and feedback. */
	UFUNCTION(Exec)
	void ChargeUltimate(float Pct);

private:

	/** Helper to retrieve the ASC of the local player executing this command. */
	UCrashAbilitySystemComponent* GetLocalASC() const;
};
