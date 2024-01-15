// Copyright Samuel Reitich 2024.

#pragma once

#include "InputActionValue.h"

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ChallengerBase.generated.h"

class UCrashInputActionMapping;
class UCrashInputComponent;
class UInputMappingContext;

class UCameraComponent;
class USkeletalMeshComponent;

UCLASS()
class PROJECTCRASH_API AChallengerBase : public ACharacter
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	AChallengerBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());



	// Character components.

// Public accessors.
public:

	/** Accessor for this character's first-person camera component. */
	UFUNCTION(Category = "Characters|Challenger|Components")
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	/** Accessor for this character's first-person mesh component. */
	UFUNCTION(Category = "Characters|Challenger|Components")
	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }
	
	/** Accessor for this character's third-person mesh component. */
	UFUNCTION(Category = "Characters|Challenger|Components")
	USkeletalMeshComponent* GetThirdPersonMesh() const { return ThirdPersonMesh; }

// Character component sub-objects.
protected:

	/** The mesh only visible in first-person. This is only seen by the local player and spectators. */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> FirstPersonMesh;

	/** The mesh only visible in third-person. This mesh is seen by all other players, and also the local player when in a third-person perspective. */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> ThirdPersonMesh;

	/** First-person camera. */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FirstPersonCameraComponent;



	// Input.

// Components.
protected:

	// This character's input component cast to CrashInputComponent.
	UPROPERTY()
	TObjectPtr<UCrashInputComponent> CrashInputComponent;

// Action mappings.
protected:

	/** The action mapping that will be bound by default. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input)
	TObjectPtr<UCrashInputActionMapping> DefaultActionMapping;

// Context mappings.
protected:

	/** The context mapping that will be bound by default. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input)
	TArray<TObjectPtr<UInputMappingContext>> DefaultInputMappings;

// Initialization.
public:

	/** Sets up input configurations, bindings, and settings. */
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

// Handler functions for native input events.
protected:

	/** Look input using a mouse. */
	void Input_Look_Mouse(const FInputActionValue& InputActionValue);

	/** Look input using a gamepad stick. */
	void Input_Look_Stick(const FInputActionValue& InputActionValue);

	/** Applies movement input. */
	void Input_Move(const FInputActionValue& InputActionValue);
};
