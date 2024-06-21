// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/DynamicEntryBox.h"
#include "SlottedEntryBox.generated.h"

/**
 * A slot that defines a location in a layout with a gameplay tag.
 */
UCLASS()
class PROJECTCRASH_API USlottedEntryBox : public UDynamicEntryBox
{
	GENERATED_BODY()

public:

	/** The tag identifying this slot. Used to match slotted widgets to their target locations. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Slot", Meta = (Categories = "UI.Slot"))
	FGameplayTag SlotID;
};
