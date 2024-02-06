// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CrashCharacterBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FJumpedSignature);

/**
 * Base class for characters in this project. Contains universal logic used by all characters, such as custom
 * delegates.
 */
UCLASS()
class PROJECTCRASH_API ACrashCharacterBase : public ACharacter
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	ACrashCharacterBase(const FObjectInitializer& ObjectInitializer);



	// Movement.

// Jumping.
public:

	/** Broadcasts JumpedDelegate. */
	virtual void OnJumped_Implementation() override;

	/** Called when this character successfully jumps. */
	UPROPERTY(BlueprintAssignable, Category = Character)
	FJumpedSignature JumpedDelegate;

};
