// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/CrashAbilitySet.h"
#include "Engine/DataAsset.h"
#include "EquipmentSet.generated.h"

class AEquipmentActor;
class UCrashAbilitySet;
class UCrashAbilitySystemComponent;
class UEquipmentAnimationData;

/**
 * Data used to define an equipment actor created by an equipment set. Used to spawn new equipment actors when the set
 * is equipped: one for first-person and one for third-person. Purely cosmetic; equipment logic is handled by the
 * equipment set.
 */
USTRUCT(BlueprintType)
struct FEquipmentActorData
{
	GENERATED_BODY()

	/** The mesh to spawn when this equipment set is equipped. */
	UPROPERTY(EditDefaultsOnly, Meta = (AllowedClasses="/Script/Engine.SkeletalMesh, /Script/Engine.StaticMesh"))
	TObjectPtr<UStreamableRenderAsset> SpawnedMesh;

	/** The name of the bone or socket to which this actor will be attached. */
	UPROPERTY(EditDefaultsOnly)
	FName AttachSocket;

	/** The offset from the AttachSocket to apply to this actor in first-person. */
	UPROPERTY(EditDefaultsOnly, DisplayName = "Attachment Offset (First-Person)")
	FTransform AttachOffset_FPP;

	/** The offset from the attached bone or socket to apply to this actor in third-person. */
	UPROPERTY(EditDefaultsOnly, DisplayName = "Attachment Offset (Third-Person)")
	FTransform AttachOffset_TPP;
};


/**
 * Structure used to maintain references to runtime equipment set data.
 */
USTRUCT(BlueprintType, Meta = (Deprecated))
struct FEquipmentSetHandleDep
{
	GENERATED_BODY()

public:

	/** The equipment set that this handle is an instance of. */
	UPROPERTY(BlueprintReadOnly, Category = "Equipment|Handles")
	TObjectPtr<UEquipmentSet> EquipmentSetData;

	/** Reference to the actors spawned after this equipment set is equipped. */
	UPROPERTY(BlueprintReadOnly, Category = "Equipment|Handles")
	TArray<TObjectPtr<AEquipmentActor>> SpawnedActors;

	/** The ASC to which this equipment set's ability set is granted. */
	UPROPERTY(BlueprintReadOnly, Category = "Equipment|Handles")
	TObjectPtr<UCrashAbilitySystemComponent> GrantedASC;

	/** Runtime reference to the granted ability set, used to remove it when this set is unequipped. */
	FCrashAbilitySet_GrantedHandles GrantedAbilitySetHandles;
};


/**
 * "Equipment" is a collection of actors that characters persistently hold. Characters can only ever have one equipment
 * set at a time.
 *
 * Most characters have a single, unique set of equipment, such as the Soldier's RPG. Some characters may switch
 * between multiple equipment sets during runtime, such as the Knight switching between a sword and shield and a spear.
 *
 * Equipment sets can grant ability sets when equipped. This is especially useful for tying primary attacks to the
 * items that they are animated with. For example, the Knight's Sword ability is granted by the Sword and Shield
 * equipment set. Therefore, if a characters wants to use the Sword ability, they need to have the sword equipped,
 * which guarantees the sword-swing animation will always be played with the sword actor.
 */
UCLASS()
class PROJECTCRASH_API UEquipmentSet : public UDataAsset
{
	GENERATED_BODY()

public:

	/** The meshes that will be spawned to visually represent this equipment. Two objects of the AEquipmentActor class
	 * will be spawned for each mesh: one for first-person and one for third-person. */
	UPROPERTY(EditDefaultsOnly)
	TArray<FEquipmentActorData> EquipmentMeshes;

	/** The animation data used by characters while this equipment is equipped. Character animations are driven by
	 * the character's current equipment, rather than the character itself. */
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UEquipmentAnimationData> AnimationData;

	/** The ability set to grant when this equipment set is equipped. */
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UCrashAbilitySet> GrantedAbilitySet;
};
