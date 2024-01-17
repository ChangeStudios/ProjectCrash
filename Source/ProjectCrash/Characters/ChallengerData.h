// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/CrashAbilitySet.h"
#include "Engine/DataAsset.h"
#include "ChallengerData.generated.h"

class AChallengerBase;
class UCrashInputActionMapping;
class UInputMappingContext;

/**
 * Defines an input mapping context with the priority with which it should be bound.
 */
USTRUCT(BlueprintType)
struct FPrioritizedInputMappingContext
{
	GENERATED_BODY()

	/** The mapping context referenced by this data structure. */
	UPROPERTY(EditDefaultsOnly, Category = Input)
	TObjectPtr<UInputMappingContext> MappingContext = nullptr;

	/** The priority of this mapping context. */
	UPROPERTY(EditDefaultsOnly, Category = Input)
	int32 Priority = 0;
};


/**
 * Data used to define the default properties of a challenger, such as their default abilities and input mappings. Each
 * challenger defines its own challenger data asset.
 *
 * This class asset exists primarily to group data that needs to be defined for each challenger into a single data
 * asset. Otherwise, we would need to go through each challenger's properties to find and set necessary data.
 */
UCLASS(BlueprintType, Const, Meta = (DisplayName = "Challenger Data", ShortToolTip = "Data used to define the default properties of a playable character."))
class PROJECTCRASH_API UChallengerData : public UPrimaryDataAsset
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	UChallengerData(const FObjectInitializer& ObjectInitializer);



	// Ability system.

public:

	/** The default ability set granted to this character when it's created. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<UCrashAbilitySet> DefaultAbilitySet;



	// Input.

public:

	/** The action mapping that will be used to bind native and ability input actions. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input)
	TObjectPtr<UCrashInputActionMapping> DefaultActionMapping;

	/** Contexts mappings that will be bound by default with the specified priority. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input)
	TArray<FPrioritizedInputMappingContext> DefaultInputMappings;
};
