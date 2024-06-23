// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CrashCharacterMovementComponent.generated.h"

/**
 * Default character movement component for this project. Integrates with its owner's MovementAttributeSet, if one
 * exists.
 */
UCLASS()
class PROJECTCRASH_API UCrashCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	UCrashCharacterMovementComponent(const FObjectInitializer& ObjectInitializer);



	// Movement modes.

protected:

	/** Disables braking friction while airborne. */
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
};
