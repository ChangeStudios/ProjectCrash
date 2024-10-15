// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_SpawnAnimationActor.generated.h"

/**
 * Spawns an actor attached to this animation. The actor is destroyed when the notify ends.
 *
 * This notify does not intrinsically support skins, but the skin system supports skin-specific montages. This means
 * that to implement different animation actors for different skins, you can create multiple animation montages using
 * the same base character animation and simply change the animation notifies in each one.
 *
 * This (A) helps to minimize and compartmentalize the data we need for each skin, and (B) makes implementing these
 * animation actors easier, as we can preview the exact skin-specific actors being spawned in-editor, instead of
 * having to guess the animation actors' appearance when editing skin data assets.
 */
UCLASS(Const, HideCategories = Object, CollapseCategories, DisplayName = "Spawn Animation Actor", Meta = (ToolTip = "Spawns an actor attached to this animation. The actor is destroyed when the notify ends."))
class PROJECTCRASH_API UAnimNotifyState_SpawnAnimationActor : public UAnimNotifyState
{
	GENERATED_BODY()
	
	// Anim notify.

public:

	/** Default constructor. */
	UAnimNotifyState_SpawnAnimationActor();

	/** Uses the spawned mesh as this notify's name. */
	virtual FString GetNotifyName_Implementation() const override;

	/** Spawns the actor defined by this notify. */
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

	/** Destroys the animation actor when the notify ends. */
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;


	
	// Editor parameters.

protected:

	/** Static or skeletal mesh asset to spawn. Must be a skeletal mesh to use ActorAnimation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify", Meta = (AllowedClasses = "SkeletalMesh StaticMesh"))
	TObjectPtr<UStreamableRenderAsset> MeshToSpawn;

	/** Optional socket at which to attach the animation actor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify")
	FName AttachSocket;

	/** Transform with which to spawn the animation actor, relative to AttachSocket if it's set. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify", DisplayName = "Relative Transform")
	FTransform SpawnTransform;

	/** Whether this actor is being spawned for a first-person animation. Enables first-person depth rendering. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify", DisplayName = "First-Person?")
	bool bFirstPerson;

	/** Optional animation to play on the animation actor when spawned, if it's a skeletal mesh. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify", DisplayName = "Actor Animation to Play", Meta = (EditConditionHides = "true"))
	TObjectPtr<UAnimationAsset> ActorAnimation;

	/** Whether the actor animation should loop. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify", DisplayName = "Loop Actor Animation?", Meta = (EditConditionHides = "true"))
	bool ActorAnimationLoops;



	// Internals.

private:

	/** The actors spawned by this notify, that will be destroyed when the notify ends. This is an array instead of a
	 * single actor because this notify may play on a replicated montage, which will spawn a local actor for each
	 * client. */
	UPROPERTY()
	TArray<AActor*> SpawnedActors;



	// Editor.

#if WITH_EDITOR

public:

	/** Hides animation properties if the selected mesh is not a skeletal mesh. */
	virtual bool CanEditChange(const FProperty* InProperty) const override;

#endif // WITH_EDITOR
};
