// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "EquipmentDefinition.generated.h"

class UCrashAbilitySet;
class UEquipmentInstance;
class UEquipmentSkin;

/**
 * Defines a piece (e.g. a gun) or a set (e.g. a sword and shield) of equipment and its static properties.
 *
 * Equipment can be "equipped" by pawns. When equipped, equipment spawns an "equipment instance" object to hold the
 * equipment's runtime data; e.g. weapon heat. Equipment spawns (purely cosmetic) actors to visually represent it,
 * which are determined by the equipping player's skins.
 *
 * Unlike inventory items, equipment logic is not expanded in the equipment definition, but rather the equipment
 * instance. The EquipmentInstance class should be subclassed to add additional equipment-specific runtime OR static
 * data.
 */
UCLASS(BlueprintType, Const)
class PROJECTCRASH_API UEquipmentDefinition : public UDataAsset
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	UEquipmentDefinition();



	// Equipment data.

public:

	/** The equipment instance class to spawn for this equipment. The equipment instance is the tangible representation
	 * of equipped equipment, and holds the equipment's runtime data. Equipment instance can be subclassed to add
	 * additional data and logic to equipment types. */
	UPROPERTY(EditDefaultsOnly, Category = "Equipment")
	TSubclassOf<UEquipmentInstance> EquipmentInstanceClass;

	/** Ability sets granted to the equipping pawn when this equipment is equipped. These will be removed when the
	 * equipment is unequipped. */
	UPROPERTY(EditDefaultsOnly, Category = "Equipment")
	TArray<TObjectPtr<const UCrashAbilitySet>> AbilitySetsToGrant;



	// Skins.

public:

	/** This equipment's default skin data. Used when the equipping player does not have a skin for this equipment, or
	 * for equipment that does not support skins. */
	UPROPERTY(EditDefaultsOnly, Category = "Equipment|Skins")
	TObjectPtr<UEquipmentSkin> DefaultEquipmentSkin;

	/**
	 * The ID for this equipment's skin data. When this equipment is equipped, this ID will be used to retrieve the
	 * equipping player's skin for this equipment, if they have one. Otherwise, the DefaultEquipmentSkin is used.
	 *
	 * Can be left empty to always use the default equipment skin.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Equipment|Skins", Meta = (Categories = "SkinID.Equipment"))
	FGameplayTag EquipmentSkinID;
};
