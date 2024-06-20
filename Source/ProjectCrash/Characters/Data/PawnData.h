// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PawnData.generated.h"

class UCrashCameraModeBase;
class UCrashAbilitySet;
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
 * Immutable properties defining a pawn. This can be derived from to create more specific pawn properties. E.g. a
 * "Challenger" might add properties defining its appearance in the "Challenger Selection" screen.
 */
UCLASS(BlueprintType, Const, Meta = (DisplayName = "Pawn Data", ShortTooltip = "Data asset used to define a pawn."))
class PROJECTCRASH_API UPawnData : public UPrimaryDataAsset
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	UPawnData();



	// Pawn.

public:

	/** The pawn that will be spawned and possessed for controllers using this pawn. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pawn")
	TSubclassOf<APawn> PawnClass;



	// Abilities.

public:

	/** Ability sets granted to this pawn when it's initialized. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TArray<TObjectPtr<UCrashAbilitySet>> AbilitySets;



	// Input.

public:

	/** This pawn's default actions. The game mode will likely add to this. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UCrashInputActionMapping> DefaultActionMapping;

	/** This pawn's default mapping context. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	FPrioritizedInputMappingContext DefaultMappingContext;



	// Camera.

public:

	/** The default camera mode used for this pawn, if given a camera component. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	TSubclassOf<UCrashCameraModeBase> DefaultCameraMode;
};
