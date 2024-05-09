// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/CrashAbilitySet.h"
#include "Engine/DataAsset.h"
#include "EquipmentSetDefinition.generated.h"

class AEquipmentActor;
class UCrashAbilitySystemComponent;
class UEquipmentAnimationData;
class UCrashAbilitySet;
class UEquipmentPieceDefinition;

/**
 * Represents an active instance of an equipment set.
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
	TArray<TObjectPtr<AEquipmentActor>> SpawnedEquipmentActors;

	/** The ASC to which this equipment set's ability set is granted. */
	UPROPERTY()
	TObjectPtr<UCrashAbilitySystemComponent> GrantedASC;

	/** The ability set currently granted by this equipment set. */
	FCrashAbilitySet_GrantedHandles GrantedAbilitySetHandle;
};



/**
 * A collection of "equipment pieces," represented by actors, that characters can persistently hold. Characters can
 * only have one equipment set at a time. Equipment sets grant abilities and drive characters' animation.
 */
UCLASS()
class PROJECTCRASH_API UEquipmentSetDefinition : public UDataAsset
{
	GENERATED_BODY()
	
public:

	/** The pieces of equipment that comprise this equipment set. These are purely cosmetic; they represent the
	 * equipment in-game. */
	UPROPERTY(EditDefaultsOnly, DisplayName = "Pieces")
	TArray<TSubclassOf<UEquipmentPieceDefinition>> EquipmentPieces;

	/** The ability set granted to an actor when they equip this set, removed when they unequip it. */
	UPROPERTY(EditDefaultsOnly, DisplayName = "Ability Set")
	TObjectPtr<UCrashAbilitySet> GrantedAbilitySet;

	UPROPERTY(EditDefaultsOnly, DisplayName = "Animation Set")
	TObjectPtr<UEquipmentAnimationData> AnimationData;
};
