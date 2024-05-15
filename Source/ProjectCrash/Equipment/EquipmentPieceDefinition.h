// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Characters/CrashCharacterBase.h"
#include "Engine/DataAsset.h"
#include "EquipmentPieceDefinition.generated.h"

/**
 * Parameters for spawning an effect on an equipment piece.
 */
USTRUCT(BlueprintType)
struct FEquipmentEffect
{
	GENERATED_BODY()

	/** Whether to attach the effect to a socket on the equipment piece's mesh. */
	UPROPERTY(EditDefaultsOnly)
	bool bAttachToSocket;

	/** The socket on the equipment mesh at which the effect will play. */
	UPROPERTY(EditDefaultsOnly)
	FName Socket;

	/** The offset from the equipment mesh's socket (or root) at which the effect will play. */
	UPROPERTY(EditDefaultsOnly)
	FVector Offset;

	/** Whether this effect should be triggered on the first-person or the third-person equipment actor. Effects
	 * with no designated perspective will be triggered on both actors. These should be used to create data-driven
	 * effects, such as those that change depending on a selected skin. Otherwise, traditional effects should be used,
	 * such as replicated gameplay cues. */
	UPROPERTY(EditDefaultsOnly, Meta = (Categories = "State.Perspective"))
	FGameplayTag EffectPerspective;

	/** Gameplay cue to trigger for the owning when this effect is activated. */
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Cue", Meta = (Categories = "GameplayCue"))
	FGameplayTag GameplayCue;

	/** Whether this gameplay cue should be added on activation. If true, the cue must be manually removed. Active
	 * cues are automatically removed when their responsible equipment set is removed. If false, the cue will be
	 * triggered via an execution. */
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Cue", DisplayName = "Add Gameplay Cue")
	bool bAddCue;

	/** Optional animation to play on the equipment actor's mesh when this effect is triggered. Only valid for skeletal
	 * meshes. */
	UPROPERTY(EditDefaultsOnly, DisplayName = "Equipment Animation")
	TObjectPtr<UAnimMontage> MeshAnimation;
};



/**
 * A single piece of an equipment set. When an equipment set is equipped, two actors are spawned for each equipment
 * piece in the set: one for first-person and one for third-person.
 *
 * Equipment pieces are distinct from equipment sets because they can be manipulated independently. For example, the
 * Knight's default equipment set contains a sword and a shield. When the "Sword" ability is used, effects can be
 * triggered exclusively on the sword, while the shield is ignored.
 */
UCLASS()
class PROJECTCRASH_API UEquipmentPieceDefinition : public UDataAsset
{
	GENERATED_BODY()

public:

	/** The mesh spawned to visually represent this equipment piece. */
	UPROPERTY(EditDefaultsOnly, Category = "Equipment Actor", Meta = (AllowedClasses="/Script/Engine.SkeletalMesh, /Script/Engine.StaticMesh"))
	TObjectPtr<UStreamableRenderAsset> Mesh;

	/** The bone or socket to which this equipment piece's actor will be attached when spawned. */
	UPROPERTY(EditDefaultsOnly, Category = "Equipment Actor", DisplayName = "Attachment Socket")
	FName AttachSocket;

	/** The offset from the attached socket at which this equipment piece's first-person actor will be spawned. */
	UPROPERTY(EditDefaultsOnly, Category = "Equipment Actor", DisplayName = "Attachment Offset (First-Person)")
	FTransform AttachOffset_FPP;

	/** The offset from the attached socket at which this equipment piece's third-person actor will be spawned. */
	UPROPERTY(EditDefaultsOnly, Category = "Equipment Actor", DisplayName = "Attachment Offset (Third-Person)")
	FTransform AttachOffset_TPP;

	/** Effects that will be played when an equipment event is fired, if this equipment piece is equipped */
	UPROPERTY(EditDefaultsOnly, Category = "Effects", DisplayName = "Effects", Meta = (Categories = "Event.EquipmentEffect"))
	TMap<FGameplayTag, FEquipmentEffect> EffectMap;
};
