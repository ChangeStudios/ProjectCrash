// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "ChallengerSkinData.generated.h"

/**
 * Defines the mesh and spawn parameters of a single "piece" of equipment. When equipped, an equipment actor is spawned
 * for each piece in both first- and third-person.
 */
USTRUCT(BlueprintType)
struct FEquipmentPiece
{
	GENERATED_BODY()

public:

	/** The mesh representing this piece of equipment (e.g. a sword). */
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<USkeletalMesh> Mesh;

	/** The bone or socket of the owning character mesh to which this piece will be attached. "None" to attach it to
	 * the character's root. */
	UPROPERTY(EditDefaultsOnly)
	FName AttachSocket;

	/** The offset from the attached socket with which to spawn this equipment piece's first-person actor. */
	UPROPERTY(EditDefaultsOnly, DisplayName = "First-Person Offset")
	FVector Offset_FPP;

	/** The offset from the attached socket with which to spawn this equipment piece's third-person actor. */
	UPROPERTY(EditDefaultsOnly, DisplayName = "Third-Person Offset")
	FVector Offset_TPP;

	/** Animations that can be played on this mesh. Static character animations (animations used regardless of skin)
	 * can trigger equipment animations by tag. Any equipped piece with the specific tag defined in this map will play
	 * the corresponding montage. This allows one AnimNotify to trigger equipment's skin-specific animations. */
	UPROPERTY(EditDefaultsOnly, Meta = (Categories = "EquipmentAnimation"))
	TMap<FGameplayTag, TObjectPtr<UAnimMontage>> Animations;
};



/**
 * Defines cosmetic data for an equipment set.
 */
USTRUCT(BlueprintType)
struct FEquipmentSetSkinData
{
	GENERATED_BODY()

public:

	/** The meshes spawned to represent this equipment set. */
	UPROPERTY(EditDefaultsOnly)
	TArray<FEquipmentPiece> Pieces;

	/** The pose used as the base for first-person character animations while this equipment set is equipped. All other
	 * equipment-based character animations are additive. */
	UPROPERTY(EditDefaultsOnly, DisplayName = "First-Person Base Pose")
	TObjectPtr<UAnimSequenceBase> BasePose_FPP;

	/** The pose used as the base for third-person character animations while this equipment set is equipped. All other
	 * equipment-based character animations are additive. */
	UPROPERTY(EditDefaultsOnly, DisplayName = "Third-Person Base Pose")
	TObjectPtr<UAnimSequenceBase> BasePose_TPP;

	/** Montage played in first-person when equipping this set. */
	UPROPERTY(EditDefaultsOnly, DisplayName = "First-Person Equip Animation")
	TObjectPtr<UAnimMontage> EquipAnim_FPP;

	/** Montage played in third-person when equipping this set. */
	UPROPERTY(EditDefaultsOnly, DisplayName = "Third-Person Equip Animation")
	TObjectPtr<UAnimMontage> EquipAnim_TPP;
};



/**
 * Defines a cosmetic Challenger skin. Whenever a player controls a Challenger, they must have a corresponding skin
 * equipped for it, even if it's just the default skin.
 */
UCLASS(BlueprintType, Const, Meta = (DisplayName = "Skin Data", ShortToolTip = "Data used to define a cosmetic Challenger skin."))
class PROJECTCRASH_API UChallengerSkinData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:

	/** The character mesh used for this skin. */
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<USkeletalMesh> CharacterMesh;

	/** Collection of skin data to use when any given equipment set is equipped by this character. If an equipment set
	 * that is not defined here is equipped by this character, it will use default skin data. */
	UPROPERTY(EditDefaultsOnly, Meta = (Categories = "EquipmentSet"))
	TMap<FGameplayTag, FEquipmentSetSkinData> EquipmentSetSkins;

	/**
	 * Gameplay cues overriding cues used by this character's abilities. When this character activates an ability that
	 * triggers a gameplay cue, any sub-tag of that cue listed here will activate instead.
	 *
	 * E.g. if an ability triggers GameplayCue.Knight.Sword.Impact and this list contains
	 * GameplayCue.Knight.Sword.Impact.MySkin, the latter will be triggered instead.
	 */
	UPROPERTY(EditDefaultsOnly, Meta = (Categories = "GameplayCue"))
	TArray<FGameplayTag> AbilityEffects;

	/** Profile image used in the Player Info widget in the player HUD. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UTexture2D> ChallengerProfile;
};
