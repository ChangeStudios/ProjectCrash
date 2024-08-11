// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EquipmentActor.generated.h"

/**
 * An actor that visually represents a piece of equipment while it is equipped. This class integrates with the
 * first-person camera system, so it syncs its visibility with the current camera mode. It also updates the equipment's
 * materials for the appropriate perspective rendering (i.e. changing depth rendering for first-person).
 *
 * Two equipment actors are spawned for any equipment piece: one for first-person and one for third-person. Events are
 * routed to both equipment actors, so they stay in visually synchronized.
 */
UCLASS(Abstract)
class PROJECTCRASH_API AEquipmentActor : public AActor
{
	GENERATED_BODY()

	/** Whether this is the first-peron or third-person actor for the equipment piece it represents. */
	bool bFirstPerson = false;
};
