// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonPlayerController.h"
#include "CrashPlayerController.generated.h"

class UCrashAbilitySystemComponent;
class ACrashPlayerState;

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



	// Input.

public:

	/** Instructs this player's ASC to process ability input. */
	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;



	// Utils.

public:

	/** Retrieves this player controller's typed player state. */
	UFUNCTION(BlueprintCallable, Category = "Crash|PlayerController")
	ACrashPlayerState* GetCrashPlayerState() const;

	/** Retrieves this player's ASC, assuming it's stored on their player state. */
	UFUNCTION(BlueprintCallable, Category = "Crash|PlayerController")
	UCrashAbilitySystemComponent* GetCrashAbilitySystemComponent() const;
};
