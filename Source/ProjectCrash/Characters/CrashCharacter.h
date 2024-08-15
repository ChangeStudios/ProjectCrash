// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "ModularCharacter.h"
#include "Camera/ViewTargetInterface.h"
#include "GameFramework/Teams/CrashTeamAgentInterface.h"
#include "CrashCharacter.generated.h"

class UCrashCameraComponent;
class UCrashAbilitySystemComponent;
class UEquipmentComponent;
class UHealthComponent;
class UPawnExtensionComponent;

/**
 * Base modular character class for this project.
 *
 * Provides first-person and third-person perspective functionality via dual mesh components. Contains a camera
 * component intended for the "camera mode" system. Provides a health component and an interface with the ability
 * system. Contains a pawn extension component for coordinating modular actor components.
 *
 * This character implements the team interface using its controller. Its team can be driven by a player controller or
 * an AI controller, assuming they implement the team interface.
 *
 * New behavior should be implemented modularly via actor components; specifically, UPawnComponent. 
 */
UCLASS(Meta = (ShortToolTip = "Base modular character class for this project."))
class PROJECTCRASH_API ACrashCharacter : public AModularCharacter, public IAbilitySystemInterface, public ICrashTeamAgentInterface, public IViewTargetInterface
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	ACrashCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());



	// Events.

// Pawn extension routing.
protected:

	/** Refreshes this character's team with the new controller and routes this event to the pawn extension component
	 * for its initialization. */
	virtual void PossessedBy(AController* NewController) override;

	/** Routes this event to the pawn extension component for its initialization. */
	virtual void UnPossessed() override;

	/** Updates this character's team when it's assigned a controller, since its controller determines its team. */
	virtual void NotifyControllerChanged() override;

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

public:

	/** Switches to third-person when starting to blend out of a first-person camera. Propagates new visibility to any
	 * equipped actors. */
	virtual void OnStartCameraModeBlendIn(UCrashCameraModeBase* PreviousCameraMode, UCrashCameraModeBase* NewCameraMode) override;

	/** Switches to first-person when fully blending into a first-person camera. Propagates new visibility to any
	 * equipped actors. */
	virtual void OnFinishCameraModeBlendIn(UCrashCameraModeBase* PreviousCameraMode, UCrashCameraModeBase* NewCameraMode) override;
	
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



	// Teams.

public:

	/** This character's team cannot be set directly. Its team is driven by its controller. */
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamId) override;

	/** Returns this character's team, driven by its controller.  */
	virtual FGenericTeamId GetGenericTeamId() const override { return TeamId_Internal; }

	/** Returns the delegate fired when this character's team is assigned or changes. */
	virtual FTeamIdChangedSignature* GetTeamIdChangedDelegate() override { return &TeamChangedDelegate; }

protected:

	/**
	 * Determines which team this character is assigned to when it's unpossessed, since its team is driven by its
	 * controller.
	 *
	 * Default implementation keeps this character on the same team (e.g. if a player is temporarily unpossessing their
	 * pawn), but could be changed to move this character to "NoTeam," a specific "neutral" team, etc.
	 */
	virtual FGenericTeamId DetermineNewTeamAfterPossessionEnds(FGenericTeamId OldTeamId) const { return OldTeamId; }

private:

	/** Fired when this character changes teams, either because it changes controllers or because its controller
	 * changed teams. */
	UPROPERTY()
	FTeamIdChangedSignature TeamChangedDelegate;

	/** Updates this character's team with its controller's team when it changes. */
	UFUNCTION()
	void OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);

	/** This character's current team, driven by their controller, cached and replicated for convenience. */
	UPROPERTY(ReplicatedUsing = OnRep_TeamId_Internal)
	FGenericTeamId TeamId_Internal;

	/** Broadcasts TeamChangedDelegate when this character's team is assigned or changes. */
	UFUNCTION()
	void OnRep_TeamId_Internal(FGenericTeamId OldTeamId);
};
