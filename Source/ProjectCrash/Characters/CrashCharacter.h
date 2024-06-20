// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "ModularCharacter.h"
#include "CrashCharacter.generated.h"

class UCrashCameraComponent;
class UCrashAbilitySystemComponent;
class UHealthComponent;
class UPawnExtensionComponent;

/**
 * Base modular character class for this project. Provides first- and third-person perspective functionality via
 * dual mesh components; contains a camera component; provides a health component and an interface with the ability
 * system; contains a pawn extension component for coordinating modular actor components.
 *
 * New behavior should be implemented modularly via actor components; specifically, UPawnComponent. 
 */
UCLASS()
class PROJECTCRASH_API ACrashCharacter : public AModularCharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	ACrashCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());



	// Initialization.

protected:

	/** Routes this event to the pawn extension component for its initialization. */
	virtual void PossessedBy(AController* NewController) override;

	/** Routes this event to the pawn extension component for its initialization. */
	virtual void UnPossessed() override;

	/** Routes this event to the pawn extension component for its initialization. */
	virtual void OnRep_Controller() override;

	/** Routes this event to the pawn extension component for its initialization. */
	virtual void OnRep_PlayerState() override;

	/** Routes this event to the pawn extension component for its initialization. */
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;



	// Character meshes.

public:

	/** Getter for the first-person mesh component. */
	FORCEINLINE USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }

	/** Getter for the third-person mesh component. */
	FORCEINLINE USkeletalMeshComponent* GetThirdPersonMesh() const { return ThirdPersonMesh; }

private:

	/** Mesh component only visible in first-person. Determined by current camera mode. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> FirstPersonMesh;

	/** Mesh component only visible in third-person. Determined by current camera mode. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> ThirdPersonMesh;



	// Camera.

protected:

	/** This character's camera. Requires a pawn camera manager component to manage its camera modes. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crash|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCrashCameraComponent> CameraComponent;



	// Pawn extension.

protected:

	/** Coordinates initialization of this actor's ASC, and its other components. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crash|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPawnExtensionComponent> PawnExtComp;



	// Ability system.

public:

	/** Typed getter for this character's ASC via its pawn extension component. */
	UFUNCTION(BlueprintCallable, Category = "Crash|Character", Meta = (ToolTip = "This character's ability system component."))
	UCrashAbilitySystemComponent* GetCrashAbilitySystemComponent() const;

	/** Interfaced getter for this character's ASC via its pawn extension component. */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:

	/** Registers this character's health component with the ASC. */
	virtual void OnAbilitySystemInitialized();

	/** Unregisters this character's health component from the ASC. */
	virtual void OnAbilitySystemUninitialized();

private:

	/** Helper component for routing events from the ability system's health attributes to the character. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crash|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHealthComponent> HealthComponent;
};
