// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "GameplayEffect.h"
#include "UIExtensionSystem.h"
#include "CrashGameplayEffectMessage.generated.h"

/**
 * A message used to communicate events related gameplay effects, primarily for widgets.
 */
USTRUCT(BlueprintType)
struct FCrashGameplayEffectMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<const UGameplayEffect> GameplayEffectDefinition;

	/** */
	UPROPERTY(BlueprintReadWrite)
	float Duration = 0.0f;

	/** */
	UPROPERTY(BlueprintReadWrite)
	int32 StackCount = 0;
};
