// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "EquipmentEventInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UEquipmentEventInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Allows this actor to receive equipment events. When an equipment event is triggered in an animation via
 * AnimNotify_TriggerEquipmentEvent, it is routed to the animation's owning actor if it implements this interface.
 */
class PROJECTCRASH_API IEquipmentEventInterface
{
	GENERATED_BODY()

public:

	/** Fired when an equipment event is routed to this actor. */
	UFUNCTION(BlueprintImplementableEvent)
	void ProcessEquipmentEvent(FGameplayTag Event);
};
