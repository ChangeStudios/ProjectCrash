// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "EquipmentActor.generated.h"

/**
 * The perspective of an equipment actor. Separate equipment actors are spawned for first- and third-person.
 */
UENUM()
enum class EEquipmentPerspective : uint8
{
	FirstPerson,
	ThirdPerson
};



/**
 * An actor that visually represents a piece of equipment while it is equipped. This class integrates with the
 * first-person camera system, so it syncs its visibility with the current camera mode. It also updates the equipment's
 * materials for the appropriate perspective rendering (i.e. changing depth rendering for first-person).
 *
 * Two equipment actors are spawned for any equipment piece: one for first-person and one for third-person. Events are
 * routed to both equipment actors, so they stay in visually synchronized.
 *
 * Note that for first-person perspective visibility to function properly, the materials used for this mesh must have a
 * FirstPersonDepthScale node in its WorldPositionOffset, which can be toggled by a scalar param named "FirstPerson."
 * This is necessary to prevent clipping in first-person.
 */
UCLASS(Abstract, HideCategories = ("Actor", "Collision", "DataLayers", "HLOD", "Input", "LevelInstance", "Physics", "Replication", "Rendering", "WorldPartition"), Meta = (PrioritizeCategories = "Equipment Tick Cooking Events")) // we don't care about any of this data
class PROJECTCRASH_API AEquipmentActor : public AActor
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	AEquipmentActor();



	// Initialization.

public:

	// Re-initializes this actor's visibility if it was initializes before being attached to its parent (a pawn's mesh).
	virtual void BeginPlay() override;



	// Perspective.

public:

	/** Returns the perspective of this equipment actor. */
	UFUNCTION(BlueprintPure, Category = "Equipment")
	EEquipmentPerspective GetEquipmentPerspective() const { return EquipmentPerspective; }

	/** Sets this equipment actor's perspective. */
	void SetEquipmentPerspective(EEquipmentPerspective InEquipmentPerspective);

protected:

	/** Whether this is the first-peron or third-person actor for the equipment piece it represents. Equipment actors
	 * are only visible to players with the same perspective as them. */
	UPROPERTY(ReplicatedUsing = "OnRep_EquipmentPerspective")
	EEquipmentPerspective EquipmentPerspective = EEquipmentPerspective::ThirdPerson;

	/** Initializes this actor's visibility to match the visibility of the component to which it's attached (e.g. hiding
	 * this actor if it's attached to a hidden component). The equipping pawn's camera logic should handle any
	 * additional visibility logic itself, such as the player's perspective changing during runtime.
	 * @see CrashCharacter::OnStartCameraModeBlendIn. */
	UFUNCTION()
	void OnRep_EquipmentPerspective();



	// Animations.

protected:

	/**
	 * Animations which can be played on this equipment actor (NOT the owning character) with a gameplay tag. Equipment
	 * actors of the same equipment piece (but different skins) should use the same tags with different animations, so
	 * the skin-specific animation can be triggered with an agnostic gameplay tag, without needing a reference to the
	 * skin or animation.
	 *
	 * By default, these animations are played on any skeletal mesh component on this actor. To manually handle an
	 * animation instead (e.g. playing different animations on multiple skeletal meshes), exclude the tag from this map,
	 * and perform the desired logic when the tag is sent as a parameter to ProcessEquipmentAnimation. All equipment
	 * animations are routed to ProcessEquipmentAnimation, even if they aren't in this map.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Equipment", Meta = (Categories = "SkinProperty"))
	TMap<FGameplayTag, TObjectPtr<UAnimMontage>> EquipmentAnimations;



	// Equipment events.

public:

	/**
	 * Processes a given animation on this equipment actor. Can be overridden in code or blueprints to manually handle
	 * animations.
	 *
	 * Default implementation searches for a matching tag in EquipmentAnimations and plays the animation on any
	 * skeletal meshes on this actor.
	 */
	virtual void ProcessEquipmentAnimation(FGameplayTag AnimationTag, EEquipmentPerspective Perspective);

protected:

	/** Blueprint-implementable function for processing equipment animations. Always called by
	 * ProcessEquipmentAnimation, even if no animation played. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Equipment", DisplayName = "ProcessEquipmentAnimation", Meta = (ToolTip = "Called when an equipment animation request is sent to this equipment actor. This can be used to manually handle equipment animations on a per-animation basis by checking the AnimationTag parameter. To manually handle an equipment animation, exclude it from EquipmentAnimations and implement the desired logic here, after checking the AnimationTag parameter."))
	void K2_ProcessEquipmentAnimation(FGameplayTag AnimationTag);
};
