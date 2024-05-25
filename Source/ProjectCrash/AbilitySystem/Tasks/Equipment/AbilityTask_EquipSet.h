// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_EquipSet.generated.h"

/**
 * Predictively equips a given equipment set through the owner's equipment component. Only non-gameplay effects are
 * predicted.
 *
 * Predicted:
 *		* Updating character animation data.
 *		* Spawning equipment actors.
 *		* Destroying equipment actors (the ones that were previously equipped).
 *		* Playing animations on equipment actors (and notifies triggered by them).
 *
 * Not Predicted:
 *		* Granting abilities.
 *
 * This task works by locally unequipping the current set, destroying its actors, and equipping the new set, spawning
 * its actors. When animations are played through the equipment system, they will be played on the predicted actors.
 * When the server spawns its own equipment actors, the predicting client will sync with the new actors, and the
 * predicted actors will be destroyed.
 *
 * If the prediction misses, then the previous set, which was predictively unequipped, will be instantly re-equipped.
 */
UCLASS()
class PROJECTCRASH_API UAbilityTask_EquipSet : public UAbilityTask
{
	GENERATED_BODY()
	
	
	
	
};
