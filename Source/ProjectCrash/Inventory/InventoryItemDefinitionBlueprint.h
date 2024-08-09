// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Blueprint.h"
#include "InventoryItemDefinitionBlueprint.generated.h"

/**
 * A specialized blueprint specifically for inventory item definitions.
 */
UCLASS(BlueprintType)
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
