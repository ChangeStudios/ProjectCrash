// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#include "CrashVerbMessage.generated.h"

/**
 * Represents a generic message of the form "{Instigator} {Verbs} {Target} {in Context}, {with Magnitude}."
 *
 * For example, "Player1 [Instigator] Damages [Verb] Player2 [Target] with 20.0 [Magnitude]."
 */
USTRUCT(BlueprintType)
struct FCrashVerbMessage
{
	GENERATED_BODY()

	/** The verb of this message. E.g. "damage dealt." */
	UPROPERTY(BlueprintReadWrite, Category=Gameplay)
	FGameplayTag Verb;

	/** The instigator of the verb of this message. */
	UPROPERTY(BlueprintReadWrite, Category=Gameplay)
	TObjectPtr<UObject> Instigator = nullptr;

	/** The target of the verb of this message. */
	UPROPERTY(BlueprintReadWrite, Category=Gameplay)
	TObjectPtr<UObject> Target = nullptr;

	/** Tags owned by the instigator. */
	UPROPERTY(BlueprintReadWrite, Category=Gameplay)
	FGameplayTagContainer InstigatorTags;

	/** Tags owned by the target. */
	UPROPERTY(BlueprintReadWrite, Category=Gameplay)
	FGameplayTagContainer TargetTags;

	/** Optional context tags for this message. */
	UPROPERTY(BlueprintReadWrite, Category=Gameplay)
	FGameplayTagContainer ContextTags;

	/** Optional numeric payload for this message's verb. E.g. amount of damage dealt. */
	UPROPERTY(BlueprintReadWrite, Category=Gameplay)
	double Magnitude = 1.0;

	/** Returns a debug string representation of this message. */
	PROJECTCRASH_API FString ToString() const;
};
