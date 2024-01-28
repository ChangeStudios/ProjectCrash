// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/CrashAbilitySet.h"
#include "Engine/DataAsset.h"
#include "EquipmentSet.generated.h"

class UCrashAbilitySet;
class UEquipmentAnimationData;


/**
 * An actor class reference for first-person and another for third-person.
 */
USTRUCT(BlueprintType)
struct FActorPair
{
	GENERATED_BODY()

	/** The first-person actor to spawn when this equipment set is equipped. Primitives should only be visible to the
	 * owner. */
	UPROPERTY(EditDefaultsOnly, DisplayName = "First-Person")
	TSubclassOf<AActor> ActorToSpawn_FPP;

	/** The third-person actor to spawn when this equipment set is equipped. Primitives should not be visible to the
	 * owner. */
	UPROPERTY(EditDefaultsOnly, DisplayName = "Third-Person")
	TSubclassOf<AActor> ActorToSpawn_TPP;
};


/**
 * An attach socket name for first-person and another for third-person.
 */
USTRUCT(BlueprintType)
struct FAttachSocketPair
{
	GENERATED_BODY()

	/** The name of the bone or socket on the first-person character mesh to attach this actor to. */
	UPROPERTY(EditDefaultsOnly, DisplayName = "First-Person")
	FName AttachSocket_FPP;

	/** The name of the bone or socket on the third-person character mesh to attach this actor to. */
	UPROPERTY(EditDefaultsOnly, DisplayName = "Third-Person")
	FName AttachSocket_TPP;
};


/**
 * An attach offset transform for first-person and another for third-person.
 */
USTRUCT(BlueprintType)
struct FAttachOffsetPair
{
	GENERATED_BODY()

	/** The offset from the attached bone or socket to apply to this actor in first-person. */
	UPROPERTY(EditDefaultsOnly, DisplayName = "First-Person")
	FTransform AttachOffset_FPP;

	/** The offset from the attached bone or socket to apply to this actor in third-person. */
	UPROPERTY(EditDefaultsOnly, DisplayName = "Third-Person")
	FTransform AttachOffset_TPP;
};


/**
 * Data used to define an actor in an equipment set. Used to spawn the actor when the set is equipped. Purely cosmetic;
 * equipment logic is handled by the equipment set.
 */
USTRUCT(BlueprintType)
struct FEquipmentActorData
{
	GENERATED_BODY()

	/** The actors to spawn when this equipment set is equipped. */
	UPROPERTY(EditDefaultsOnly)
	FActorPair SpawnedActorClasses;

	/** The names of the bones or sockets on the character mesh to attach this actor to. */
	UPROPERTY(EditDefaultsOnly)
	FAttachSocketPair AttachSockets;

	/** The offsets from the attached bones or sockets to apply to this actor. */
	UPROPERTY(EditDefaultsOnly)
	FAttachOffsetPair AttachOffsets;
};


/**
 * Structure used to maintain references to runtime equipment set data.
 */
USTRUCT()
struct FEquipmentSetHandle
{
	GENERATED_BODY()

public:

	/** Reference to the first-person actor spawned after this equipment set is equipped. */
	UPROPERTY(BlueprintReadOnly, Category = "Equipment|Handles")
	TObjectPtr<AActor> SpawnedActor_FPP = nullptr;

	/** Reference to the third-person actor spawned after this equipment set is equipped. */
	UPROPERTY(BlueprintReadOnly, Category = "Equipment|Handles")
	TObjectPtr<AActor> SpawnedActor_TPP = nullptr;

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

	/** The actors that will be spawned to visually represent this equipment. */
	UPROPERTY(EditDefaultsOnly)
	TArray<FEquipmentActorData> EquipmentActors;

	/** The animation data used by characters while this equipment is equipped. Character animations are driven by
	 * the character's current equipment, rather than the character itself. */
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UEquipmentAnimationData> AnimationData;

	/** The ability set to grant when this equipment set is equipped. */
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UCrashAbilitySet> GrantedAbilitySet;
};
