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
 */
UCLASS(Abstract, HideCategories = ("Actor", "Collision", "DataLayers", "HLOD", "Input", "LevelInstance", "Physics", "Replication", "Rendering", "WorldPartition"), Meta = (PrioritizeCategories = "Equipment Tick Cooking Events")) // we don't care about any of this data
class PROJECTCRASH_API AEquipmentActor : public AActor
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	AEquipmentActor();



	// Perspective.

public:

	/** Returns the perspective of this equipment actor. */
	UFUNCTION(BlueprintPure, Category = "Equipment")
	EEquipmentPerspective GetEquipmentPerspective() const { return EquipmentPerspective; }

	/** Sets this equipment actor's perspective and uses it to initialize this actor's visibility. Should only be
	 * called once: when this equipment actor is spawned. */
	void InitEquipmentPerspective(EEquipmentPerspective InEquipmentPerspective);

protected:

	/** Whether this is the first-peron or third-person actor for the equipment piece it represents. Equipment actors
	 * are only visible to players with the same perspective as them.
	 *
	 * NOTE: May not need this. We can just make the character hide and reveal any actors attached to it when its visibility changes.
	 */
	EEquipmentPerspective EquipmentPerspective = EEquipmentPerspective::ThirdPerson;



	// Animations.

protected:

	/**
	 * Animations which will be played on this equipment actor (NOT the owning character). Gameplay tags are used to
	 * retrieve the actor-specific (i.e. skin-specific) animation that should be used for each animation. By default,
	 * these animations are each played on any skeletal mesh component on this actor.
	 *
	 * These animations are routed through this actor through ProcessEquipmentEvent. To manually handle an animation
	 * (e.g. playing an animation on multiple meshes) or event, exclude the desired animation/event tag from this list
	 * and implement ProcessEquipmentEvent.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Equipment", Meta = (Categories = "SkinProperty"))
	TMap<FGameplayTag, TObjectPtr<UAnimMontage>> EquipmentAnimations;



	// Equipment events.

public:

	/** Processes the given event on this equipment actor. Default implementation attempts to find an animation in
	 * EquipmentAnimations with a matching tag and plays it on any skeletal mesh components on this actor. */
	virtual void ProcessEquipmentEvent(FGameplayTag Event);

protected:

	/** Blueprint-implementable function for manually handling equipment events. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Equipment", Meta = (ToolTip = "Called when an equipment event is routed to this equipment actor."))
	void K2_ProcessEquipmentEvent(FGameplayTag Event);
};
