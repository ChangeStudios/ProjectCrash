// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "CrashCharacterBase.h"
#include "NPCBase.generated.h"

class UAbilitySystemExtensionComponent;
class UCrashAbilitySystemComponent;
class UHealthAttributeSet;
class UHealthComponent;
class UNPCData;

/**
 * Base non-player character class. These are AI characters that have basic ability system functionality, capable of
 * taking damage, getting gameplay effects, etc.
 */
UCLASS(Abstract)
class PROJECTCRASH_API ANPCBase : public ACrashCharacterBase, public IAbilitySystemInterface
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	ANPCBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());



	// Initialization.

protected:

	/** Initializes this character's ASC after its other components have been initialized. */
	virtual void PostInitializeComponents() override;



	// Data.

public:

	/** A collection of data used to define the default properties of this challenger, such as its default abilities
	 * and input mappings. */
	UPROPERTY(EditDefaultsOnly, Category = "NPC Data")
	TObjectPtr<const UNPCData> NPCData;



	// Ability system.

// Utils.
public:

	/** Interfaced accessor that retrieves this character's ASC. */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/** Retrieves this character's ASC cast to the UCrashAbilitySystemComponent class. Returns nullptr if the ASC does
	 * not exist or if the ASC is not of the correct class. Uses the ability system interface to find the ASC. */
	UFUNCTION(BlueprintPure, Category = "Ability System") 
	UCrashAbilitySystemComponent* GetCrashAbilitySystemComponent() const;

// Initialization.
protected:

	/** Callback after this character becomes the avatar of an ASC that initializes its attribute sets. */
	virtual void OnAbilitySystemInitialized();

	/** Callback bound to when this character is uninitialized as the avatar of an ASC that initializes its attribute
	 * sets. */
	virtual void OnAbilitySystemUninitialized();

// Components.
protected:

	/** This character's ability system component. */
	UPROPERTY(VisibleAnywhere, Category = "Ability System")
	TObjectPtr<UCrashAbilitySystemComponent> AbilitySystemComponent;

	/** Handles initialization and uninitialization of an ability system component with this character when it is possessed
	 * and unpossessed/destroyed. */
	TObjectPtr<UAbilitySystemExtensionComponent> ASCExtensionComponent;



	// Attributes.

// Attribute sets.
protected:

	/** This character's health attribute set. */
	UPROPERTY()
	TObjectPtr<UHealthAttributeSet> HealthSet;

// Components.
protected:

	/** This character's health component. Acts as an interface to this character's ASC's health attribute set. Gets
	 * initialized with an ASC after one is initialized with this character. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability System|Attributes", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHealthComponent> HealthComponent;
};
