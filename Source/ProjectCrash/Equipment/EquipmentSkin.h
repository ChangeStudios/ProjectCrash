// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "EquipmentSkin.generated.h"

class AEquipmentActor;

/**
 * Data used to define an equipment actor that will be spawned when equipment is equipped.
 */
USTRUCT(BlueprintType)
struct FEquipmentSkinActorData
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

	/** Skin-specific animations which will be played on this equipment actor (NOT the owning character). Gameplay tags
	* are used to retrieve the skin-specific animation that should be used for each animation. */
	UPROPERTY(EditDefaultsOnly, Category = "Equipment", Meta = (Categories = "SkinProperty"))
	TMap<FGameplayTag, TObjectPtr<UAnimMontage>> EquipmentAnimations;
};



/**
 * Defines cosmetic ("skin") data for a piece or collection of equipment. When equipment is equipped, this data is used
 * to spawn a visual representation of the equipment, in both first- and third-person. This data also defines additional
 * cosmetic equipment data, such as the animations used for this skin.
 */
UCLASS(BlueprintType, Const)
class PROJECTCRASH_API UEquipmentSkin : public UDataAsset
{
	GENERATED_BODY()

public:

	/** The equipment actors to spawn in first-person. These are only spawned if the equipping pawn is of type
	 * CrashCharacter, and are attached to the first-person mesh. */
	UPROPERTY(EditDefaultsOnly, Category = "Equipment Actors")
	TArray<FEquipmentSkinActorData> FirstPersonActorsToSpawn;

	/** The equipment actors to spawn in third-person. */
	UPROPERTY(EditDefaultsOnly, Category = "Equipment Actors")
	TArray<FEquipmentSkinActorData> ThirdPersonActorsToSpawn;



	// Animation.

public:

	/** The animation blueprint to use for first-person meshes while this equipment is equipped with this skin. */
	UPROPERTY(EditDefaultsOnly, Category = "Animation", DisplayName = "First-Person Animation Blueprint")
	TSubclassOf<UAnimInstance> FirstPersonAnimInstance;

	/** The animation blueprint to use for third-person meshes while this equipment is equipped with this skin. */
	UPROPERTY(EditDefaultsOnly, Category = "Animation", DisplayName = "Third-Person Animation Blueprint")
	TSubclassOf<UAnimInstance> ThirdPersonAnimInstance;
};
