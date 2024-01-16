// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CrashCharacterMovementComponent.generated.h"

/**
 * Default character movement component for this project. Integrates movement properties, such as movement speed,
 * into the gameplay ability system. 
 */
UCLASS()
class PROJECTCRASH_API UCrashCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	UCrashCharacterMovementComponent(const FObjectInitializer& ObjectInitializer);
};
