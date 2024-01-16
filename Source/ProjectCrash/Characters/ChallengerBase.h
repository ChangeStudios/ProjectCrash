// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "InputActionValue.h"
#include "GameFramework/Character.h"
#include "ChallengerBase.generated.h"

class UAbilitySystemExtensionComponent;
class UCameraComponent;
class UChallengerData;
class UCrashAbilitySystemComponent;
class UCrashInputActionMapping;
class UCrashInputComponent;
class UInputMappingContext;
class USkeletalMeshComponent;

/**
 * The base class for all playable characters (a.k.a. "challengers"). Contains universal player character functionality
 * such as a first-person camera, input, and an interface with the ability system.
 *
 * This class should be derived from for each playable character and never used directly.
 */
UCLASS(Abstract, meta = (PrioritizeCategories = "Challenger Data"))
class PROJECTCRASH_API AChallengerBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	AChallengerBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());



	// Initialization.

protected:

	/** Performs server-side initialization for the owning player's ASC. */
	virtual void PossessedBy(AController* NewController) override;

	/** Performs client-side initialization for the owning player's ASC. */
	virtual void OnRep_PlayerState() override;



	// Data.

public:

	/** A collection of data used to define the default properties of this challenger, such as its default abilities
	 * and input mappings. */
	UPROPERTY(EditDefaultsOnly, Category = "Challenger Data")
	TObjectPtr<const UChallengerData> ChallengerData;



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



	// Ability system.

// Utils.
public:

	/** Interfaced accessor retrieves this character's bound ASC. */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/** Retrieves this character's bound ASC. */
	UFUNCTION(BlueprintPure, Category = "Ability System")
	UCrashAbilitySystemComponent* GetCrashAbilitySystemComponent() const;

// Initialization.
protected:

	/** Callback after this pawn becomes the avatar of an ASC that sets up . */
	virtual void OnAbilitySystemInitialized();

	/** Callback bound to when this character is uninitialized as the avatar of an ASC. */
	virtual void OnAbilitySystemUninitialized();

// Components.
protected:
	
	/** Handles initialization and uninitialization of the ability system with this pawn. */
	TObjectPtr<UAbilitySystemExtensionComponent> ASCExtensionComponent;



	// Input.

// Components.
protected:

	// This character's input component cast to CrashInputComponent.
	UPROPERTY()
	TObjectPtr<UCrashInputComponent> CrashInputComponent;

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