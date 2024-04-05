// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/CrashAbilitySet.h"
#include "Engine/DataAsset.h"
#include "ChallengerData.generated.h"

class AChallengerBase;
class UCharacterAnimData;
class UCrashInputActionMapping;
class UEquipmentSet;
class UHealthAttributeBaseValues;
class UInputMappingContext;
class ULevelSequence;

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
 * Classes that categorize the broad play-style of each Challenger. 
 */
UENUM(BlueprintType)
enum class EChallengerClass : uint8
{
	None,

	Brawler,
	Hybrid,
	Mage
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



	// Actors.

public:

	/** The class of the pawn that will be spawned and possessed for players playing this challenger. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Actors")
	TSubclassOf<AChallengerBase> PawnClass;

	/** The class of the actor that will be spawned in the character selection screen when this character is
	 * selected. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Actors")
	TSubclassOf<AActor> CharacterSelectionActor;



	// Ability system.

public:

	/** The default ability set granted to this character when it's created. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<UCrashAbilitySet> DefaultAbilitySet;



	// Attributes.

public:

	/** The default values to which health attributes are initialized. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
	TObjectPtr<UHealthAttributeBaseValues> HealthAttributeBaseValues;



	// Equipment.

public:

	/** The default equipment set granted to this character when it's created. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<UEquipmentSet> DefaultEquipmentSet;



	// Animation.

public:

	/** This character's default animation data. This should never be used; it should immediately be overridden by the
	 * animation data of this character's default equipment set. This serves as a safety net if the character ends up
	 * without an equipment set for some reason. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UCharacterAnimData> DefaultAnimData;

	/** The level sequence that plays on the player camera when this character is selected in the character selection
	 * screen. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<ULevelSequence> CharacterSelectionSequence;



	// Input.

public:

	/** The action mapping that will be used to bind native and ability input actions. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input)
	TObjectPtr<UCrashInputActionMapping> DefaultActionMapping;

	/** Contexts mappings that will be bound by default with the specified priority. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input)
	TArray<FPrioritizedInputMappingContext> DefaultInputMappings;



	// UI.

public:

	/** This Challenger's user-facing name. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "User Interface")
	FString ChallengerDisplayName;

	/** This Challenger's user-facing class. Serves to categorize the play-style of this Challenger for players. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "User Interface")
	EChallengerClass ChallengerClass;

	/** The image that appears in the HUD to represent this Challenger. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "User Interface")
	TObjectPtr<UTexture2D> ProfileImage;

	/** The image that appears in the character selection screen in the button representing this Challenger. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "User Interface")
	TObjectPtr<UTexture2D> SelectionImage;

	/** The abilities listed in "ability info" screens for this character. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "User Interface")
	TArray<TSubclassOf<UCrashGameplayAbilityBase>> ListedAbilities;
};
