// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "CrashCharacterBase.h"
#include "GameplayTagContainer.h"
#include "InputActionValue.h"
#include "AbilitySystem/Abilities/CrashAbilitySet.h"
#include "ChallengerBase.generated.h"

class UEquipmentComponent;
class UHealthComponent;
class UAbilitySystemExtensionComponent;
class UCameraComponent;
class UChallengerData;
class UCrashAbilitySystemComponent;
class UCrashInputActionMapping;
class UCrashInputComponent;
class UInputMappingContext;
class USkeletalMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FASCInitializedSignature, UCrashAbilitySystemComponent*, CrashASC);

/**
 * The base class for all playable characters (a.k.a. "challengers"). Contains universal player character functionality
 * such as a first-person camera, input, and an interface with the ability system.
 */
UCLASS(Abstract, meta = (PrioritizeCategories = "Challenger Data"))
class PROJECTCRASH_API AChallengerBase : public ACrashCharacterBase, public IAbilitySystemInterface
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	AChallengerBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());



	// Initialization.

/* Uses the AbilitySystemExtensionComponent to initialize this character as the avatar of the owning player's ASC when
 * possessed. */
protected:

	/** Performs server-side initialization of this character with the owning player's ASC. */
	virtual void PossessedBy(AController* NewController) override;

	/** Performs client-side initialization of this character with the owning player's ASC. */
	virtual void OnRep_PlayerState() override;



	// Uninitialization.

protected:

	/** Uninitializes this character with its owning player's ASC and destroys itself. This is used when this character
	 * has already been unpossessed but still needs to be cleaned up. */
	void UninitAndDestroy();

private:

	/** Called when this character's death starts and finishes to call OnDeathStarted and OnDeathFinished. */
	UFUNCTION()
	void HandleDeathStateChanged(const FGameplayTag Tag, int32 NewCount);

protected:

	/** Called when this character's death starts, to handle logic like ragdolling. */
	void OnDeathStarted();

	/** Called when this character's death finishes, to handle logic like destruction. */
	void OnDeathFinished();



	// Challenger data.

public:

	/** Getter for ChallengerData. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Characters|Challenger|Data", meta = (ToolTip = "Data used to define the default properties of a challenger."))
	UChallengerData* GetChallengerData() const { return ChallengerData; }

protected:

	/** Data used to define the default properties of this challenger, such as its default abilities and input
	 * mappings. */
	UPROPERTY(EditDefaultsOnly, Category = "Challenger Data")
	TObjectPtr<UChallengerData> ChallengerData;



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



	// Equipment.

// Components.
protected:

	/** Allows this character to equip various equipment sets. Most challengers only ever need one equipment set, but
	 * some may need multiple. For example, if an ability grants the challenger a special weapon, they will need an
	 * additional equipment set for that weapon. */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Equipment", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UEquipmentComponent> EquipmentComponent;



	// Ability system.

// Utils.
public:

	/** Interfaced accessor that retrieves this character's bound ASC via its ability system extension component. */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/** Retrieves this character's ASC cast to the UCrashAbilitySystemComponent class. Returns nullptr if the ASC does
	 * not exist or if the ASC is not of the correct class. Uses the ability system interface to find the ASC. */
	UFUNCTION(BlueprintPure, Category = "Ability System") 
	UCrashAbilitySystemComponent* GetCrashAbilitySystemComponent() const;

protected:

	/** The handles for the default ability set currently granted by this character. Used to remove the ability set
	 * from the ASC when this character is destroyed or unpossessed. */
	FCrashAbilitySet_GrantedHandles GrantedDefaultAbilitySetHandle;

// Initialization.
protected:

	/** Callback after this character becomes the avatar of an ASC that grants its default ability set and initializes its
	 * attribute sets. */
	virtual void OnAbilitySystemInitialized();

	/** Callback bound to when this character is uninitialized as the avatar of an ASC that removes its default ability
	 * set and initializes its attribute sets. */
	virtual void OnAbilitySystemUninitialized();

public:

	/** Called when this character finishes initializing its ASC. Allows other components and actors to cache this
	 * character's ASC on startup, even if the ASC isn't ready yet. */
	UPROPERTY(BlueprintAssignable, Category = "Ability System")
	FASCInitializedSignature ASCInitializedDelegate;

// Components.
protected:
	
	/** Handles initialization and uninitialization of an ability system component with this character when it is possessed
	 * and unpossessed/destroyed. */
	TObjectPtr<UAbilitySystemExtensionComponent> ASCExtensionComponent;



	// Attributes.

// Components.
protected:

	/** This character's health component. Acts as an interface to this character's ASC's health attribute set, which
	 * is stored in the owning player's player state. Gets initialized with an ASC after one is initialized with this
	 * character. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability System|Attributes", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHealthComponent> HealthComponent;



	// Input.

// Components.
protected:

	// This character's input component cached as a CrashInputComponent.
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