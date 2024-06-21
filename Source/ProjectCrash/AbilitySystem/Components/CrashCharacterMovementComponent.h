// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CrashCharacterMovementComponent.generated.h"

class UCrashAbilitySystemComponent;

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



	// Initialization.

public:

	/** Binds relevant delegates. */
	virtual void BeginPlay() override;



	// Movement modes.

// Jumping.
protected:

	/** Adds the "Jumping" tag to this character until they land. */
	UFUNCTION()
	void OnJumped();

protected:

	/** Adds the "Falling" gameplay tag when this component's owning character begins falling. Removes the "Jumping"
	 * and "Falling" tags when landing. */
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
};
