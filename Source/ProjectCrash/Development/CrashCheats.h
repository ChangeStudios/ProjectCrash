// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "CrashCheats.generated.h"

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
};
