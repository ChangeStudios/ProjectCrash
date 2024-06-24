// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonPlayerController.h"
#include "CrashPlayerController.generated.h"

/**
 * Base modular player controller for this project.
 */
UCLASS()
class PROJECTCRASH_API ACrashPlayerController : public ACommonPlayerController
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	ACrashPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
