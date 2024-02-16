// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "CrashPlayerState.generated.h"

class UCrashAbilitySet;
class UHealthAttributeSet;
class UCrashAbilitySystemComponent;

/**
 * The default player state used for all players. Contains player data that persists past pawn death or destruction,
 * such as match stats or the currently selected challenger.
 *
 * Owns and manages the player's ability system component, allowing persistent ability data, like current ultimate
 * charge, to be safely tracked independently from player's pawn.
 */
UCLASS()
class PROJECTCRASH_API ACrashPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	ACrashPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());



	// Initialization.

public:

	/** Initializes the ASC with this player state as the owner after all other components have been initialized. */
	virtual void PostInitializeComponents() override;



	// Ability system.

// Accessors.
public:

	/** ASC interface's default getter. */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/** Typed getter for this player's ASC. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Ability System")
	UCrashAbilitySystemComponent* GetCrashAbilitySystemComponent() const { return AbilitySystemComponent; }

protected:

	/** This player's ability system component. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability System")
	TObjectPtr<UCrashAbilitySystemComponent> AbilitySystemComponent;

// Ability sets.
protected:

	/** An ability set granted to this player when their ASC is initialized and never (automatically) removed. */
	UPROPERTY(EditDefaultsOnly, Category = "Ability System")
	TObjectPtr<UCrashAbilitySet> PlayerAbilitySet;

// Attribute sets. These have to be created in the same class as their ASC.
protected:

	/** This player's health attribute set. */
	UPROPERTY()
	TObjectPtr<UHealthAttributeSet> HealthSet;

// Callbacks.
private:
	/**
	 * Callback bound to when this player's ASC gains or loses an InputBlocking gameplay tag. Disables or enables
	 * movement and camera input, respectively.
	 * 
	 * @param Tag			InputBlocking tag.
	 * @param NewCount		Number of InputBlocking tags the ASC now has.
	 */
	UFUNCTION()
	void OnInputBlockingChanged(const FGameplayTag Tag, int32 NewCount);

	/** Delegate to bind the OnInputBlockingChanged callback. */
	FDelegateHandle InputBlockingDelegate;
};
