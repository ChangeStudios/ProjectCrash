// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Blueprint.h"
#include "CrashCameraModeBlueprint.generated.h"

/**
 * A specialized blueprint specifically for camera modes.
 */
UCLASS(BlueprintType)
class PROJECTCRASH_API UCrashCameraModeBlueprint : public UBlueprint
{
	GENERATED_BODY()

	UCrashCameraModeBlueprint(const FObjectInitializer& ObjectInitializer);

#if WITH_EDITOR

	// UBlueprint interface
	virtual bool SupportedByDefaultBlueprintFactory() const override
	{
		return false;
	}
	// End of UBlueprint interface

#endif

};
