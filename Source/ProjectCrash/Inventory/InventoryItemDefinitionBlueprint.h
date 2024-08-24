// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Blueprint.h"
#include "InventoryItemDefinitionBlueprint.generated.h"

/**
 * A specialized blueprint specifically for inventory item definitions.
 */
UCLASS(BlueprintType, Meta = (ToolTip = "Defines an item and its static properties. In-game items are instantiated from this definition.\n\nItems are defined with a collection of item \"traits.\" Each trait defines static properties of an item, such as whether it can be equipped. Items only contain the traits relevant to them. For example, if an item cannot be equipped, it will not have the \"equippable\" trait."))
class PROJECTCRASH_API UInventoryItemDefinitionBlueprint : public UBlueprint
{
	GENERATED_BODY()

	UInventoryItemDefinitionBlueprint(const FObjectInitializer& ObjectInitializer);

#if WITH_EDITOR

	// UBlueprint interface
	virtual bool SupportedByDefaultBlueprintFactory() const override
	{
		return false;
	}
	// End of UBlueprint interface

#endif

};
