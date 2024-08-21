// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "EquipmentSkin.generated.h"

class AEquipmentActor;

/**
 * Data used to define an equipment actor that will be spawned when equipment is equipped.
 */
USTRUCT(BlueprintType)
struct FEquipmentSkinActorInfo
{
	GENERATED_BODY()

	/** The actor to spawn. */
	UPROPERTY(EditDefaultsOnly, Category = "Equipment")
	TSubclassOf<AEquipmentActor> ActorToSpawn;

	/** The socket to which the actor should be attached on the equipping pawn's mesh when spawned. */
	UPROPERTY(EditDefaultsOnly, Category = "Equipment")
	FName AttachSocket;

	/** The transform with which the actor will be spawned. Transform is relative to the component to which the actor
	 * is attached. */
	UPROPERTY(EditDefaultsOnly, Category = "Equipment")
	FTransform AttachTransform;
};



/**
 * Defines cosmetic ("skin") data for a piece or collection of equipment. When equipment is equipped, this data is used
 * to spawn a visual representation of the equipment, in both first- and third-person.
 *
 * Runtime cosmetic data, such as the animations played on each equipment actor, are defined in the equipment actor
 * class itself.
 */
UCLASS(BlueprintType, Const)
class PROJECTCRASH_API UEquipmentSkin : public UDataAsset
{
	GENERATED_BODY()

public:

	/** The equipment actors to spawn in first-person. These are only spawned if the equipping pawn is of type
	 * CrashCharacter, and are attached to the first-person mesh. */
	UPROPERTY(EditDefaultsOnly, Category = "Equipment Actors")
	TArray<FEquipmentSkinActorInfo> FirstPersonActorsToSpawn;

	/** The equipment actors to spawn in third-person. */
	UPROPERTY(EditDefaultsOnly, Category = "Equipment Actors")
	TArray<FEquipmentSkinActorInfo> ThirdPersonActorsToSpawn;



	// Animation.

public:

	/** Played on the pawn's first-person mesh (if they have one) when equipment is equipped with this skin. */
	UPROPERTY(EditDefaultsOnly, Category = "Animation", DisplayName = "First-Person \"Equip\" Animation")
	TObjectPtr<UAnimMontage> FirstPersonEquipAnim;

	/** Played on the pawn's third-person mesh (if they have one) when equipment is equipped with this skin. */
	UPROPERTY(EditDefaultsOnly, Category = "Animation", DisplayName = "Third-Person \"Equip\" Animation")
	TObjectPtr<UAnimMontage> ThirdPersonEquipAnim;

	/** The animation blueprint to use for first-person meshes while this equipment is equipped with this skin. */
	UPROPERTY(EditDefaultsOnly, Category = "Animation", DisplayName = "First-Person Animation Blueprint")
	TSubclassOf<UAnimInstance> FirstPersonAnimInstance;

	/** The animation blueprint to use for third-person meshes while this equipment is equipped with this skin. */
	UPROPERTY(EditDefaultsOnly, Category = "Animation", DisplayName = "Third-Person Animation Blueprint")
	TSubclassOf<UAnimInstance> ThirdPersonAnimInstance;



	// Validation.

public:

#if WITH_EDITOR

	/** Ensures no actor info entries have a null class. */
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;

#endif // WITH_EDITOR
};
