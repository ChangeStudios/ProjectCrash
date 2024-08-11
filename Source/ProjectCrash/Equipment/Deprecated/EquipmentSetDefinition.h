// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/CrashAbilitySet.h"
#include "Characters/Data/ChallengerSkinData.h"
#include "Engine/DataAsset.h"
#include "EquipmentSetDefinition.generated.h"

class AEquipmentPieceActor;
class UCharacterAnimData;
class UCrashAbilitySet;
class UCrashAbilitySystemComponent;

/**
 * Represents an active instance of an equipment set: either a currently equipped set or a temporarily unequipped set.
 */
USTRUCT()
struct FEquipmentSetHandle
{
	GENERATED_BODY()

public:

	/** The equipment set of which this handle is an instance. */
	UPROPERTY()
	TObjectPtr<UEquipmentSetDefinition> EquipmentSetDefinition;

	/** Spawned actors representing this equipment set's pieces. Each piece in an equipped set has a first-person and
	 * third-person actor. */
	UPROPERTY()
	TArray<TObjectPtr<AEquipmentPieceActor>> SpawnedEquipmentActors;

	/** The ASC to which this equipment set's ability set is granted. */
	UPROPERTY()
	TObjectPtr<UCrashAbilitySystemComponent> GrantedASC;

	/** The ability set currently granted by this equipment set. */
	FCrashAbilitySet_GrantedHandles GrantedAbilitySetHandle;
};



/**
 * A set of equipment that can be collectively "equipped" by an actor with an EquipmentComponent. Characters can only
 * have one equipment set equipped at any time. Equipment sets drive character animation, grant abilities, and spawn
 * actors to visually represent themselves.
 *
 * TODO: Deprecate
 */
UCLASS(BlueprintType, Const, Meta = (ShortToolTip = "A set of equipment that can be equipped by any actor with an EquipmentComponent. Intended for use with CrashCharacters."))
class PROJECTCRASH_API UEquipmentSetDefinition : public UDataAsset
{
	GENERATED_BODY()

public:

	/** Identifying tag for this equipment set. Used to keep this equipment set skin-agnostic, allowing skins to use
	 * tags to define which equipment sets they may override certain properties of. */
	UPROPERTY(EditDefaultsOnly, Meta = (Categories = "EquipmentSet"))
	FGameplayTag SetID;

	// /** Animation data used by the equipping character while this equipment set is equipped. */
	// UPROPERTY(EditDefaultsOnly)
	// TObjectPtr<UCharacterAnimData> AnimationData;

	/** The ability set granted to an actor when they equip this set, removed when they fully unequip it (as opposed
	 * to temporarily unequipping it). */
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UCrashAbilitySet> GrantedAbilitySet;

	/** Equipment skin data to use if there is no data defined for this equipment set in the equipping character's skin
	 * data. This should only be used if a character for whom this set is not designed for equips this set. */
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UEquipmentSetSkinData> DefaultSkinData;
};
