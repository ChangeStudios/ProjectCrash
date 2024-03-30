// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#include "CrashVerbMessage.generated.h"

/**
 * Represents a generic message of the form "{Instigator} {Verbs} {Target} {in Context}, {with Magnitude}."
 *
 * For example, "Player1 Damages Player2 with 20.0 [damage]."
 */
USTRUCT(BlueprintType)
struct FCrashVerbMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category=Gameplay)
	FGameplayTag Verb;

	UPROPERTY(BlueprintReadWrite, Category=Gameplay)
	TObjectPtr<UObject> Instigator = nullptr;

	UPROPERTY(BlueprintReadWrite, Category=Gameplay)
	TObjectPtr<UObject> Target = nullptr;

	UPROPERTY(BlueprintReadWrite, Category=Gameplay)
	FGameplayTagContainer InstigatorTags;

	UPROPERTY(BlueprintReadWrite, Category=Gameplay)
	FGameplayTagContainer TargetTags;

	UPROPERTY(BlueprintReadWrite, Category=Gameplay)
	FGameplayTagContainer ContextTags;

	UPROPERTY(BlueprintReadWrite, Category=Gameplay)
	double Magnitude = 1.0;

	// Returns a debug string representation of this message
	PROJECTCRASH_API FString ToString() const;
};
