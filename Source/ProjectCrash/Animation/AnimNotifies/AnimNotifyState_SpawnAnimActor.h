// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_SpawnAnimActor.generated.h"

/**
 * Spawns an actor attached to this animation. The actor is destroyed when the notify ends.
 */
UCLASS(Const, HideCategories = Object, ShowCategories="AnimNotify", DisplayName = "Spawn Animation Actor")
class PROJECTCRASH_API UAnimNotifyState_SpawnAnimActor : public UAnimNotifyState
{
	GENERATED_BODY()
	
	// Anim notify.

public:

	// Default constructor.
	UAnimNotifyState_SpawnAnimActor();

	// Spawns the actor defined by this notify.
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

	// Destroys the animation actor when the notify ends.
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;



	// Editor parameters.

protected:

	// Static or skeletal mesh asset to spawn. Must be a skeletal mesh to use ActorAnimation.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify", Meta = (AllowedClasses = "/Script/Engine.SkeletalMesh /Script/Engine.StaticMesh"))
	TObjectPtr<UStreamableRenderAsset> MeshToSpawn;

	// Optional socket at which to attach the animation actor. "None" to not attach the spawned actor.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify")
	FName AttachSocket;

	// Transform with which to spawn the animation actor, relative to AttachSocket if it's set.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify", DisplayName = "Relative Transform")
	FTransform SpawnTransform;

	// Whether to override the spawned mesh's materials.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify|Material Override", DisplayName = "Enable Material Overrides")
	uint32 bOverrideMaterials : 1;

	/* The materials to use for each of the spawned mesh's material indices. Overrides the mesh's default materials.
	 * Indices without a valid material will be skipped, and revert to the mesh's default material for that index. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify|Material Override", Meta = (EditCondition = "bOverrideMaterials", EditConditionHides = "true", ScriptName = "MaterialOverrides"))
	TArray<TObjectPtr<UMaterialInterface>> OverrideMaterials;

	// The overlay material to apply to the mesh.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify|Material Override", Meta = (EditCondition = "bOverrideMaterials", EditConditionHides = "true"))
	TObjectPtr<UMaterialInterface> OverlayMaterial;

	// Optional animation to play on the animation actor when spawned, if it's a skeletal mesh.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify", DisplayName = "Actor Animation to Play", Meta = (EditConditionHides = "true"))
	TObjectPtr<UAnimationAsset> ActorAnimation;

	// Whether the actor animation should loop.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify", DisplayName = "Loop Actor Animation?", Meta = (EditConditionHides = "true"))
	bool ActorAnimationLoops;



	// Internals.

private:

	// The actor spawned by this notify, that will be destroyed when the notify ends.
	UPROPERTY()
	AActor* SpawnedActor;

	// Applies optional material overrides to the given mesh component.
	void ApplyMaterialOverrides(UMeshComponent* MeshComp);



#if WITH_EDITOR

public:

	// Hides animation properties if the selected mesh is not a skeletal mesh.
	virtual bool CanEditChange(const FProperty* InProperty) const override;

#endif // WITH_EDITOR
};
