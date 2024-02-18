// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "CrashPlayerState.generated.h"

class UHealthAttributeSet;

/**
 * The player state used during gameplay (as opposed to menus, lobbies, etc.).
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



	// Game data.

// Lives.
public:

	/** Decrements this player's current number of lives. If the player runs out of lives, the game mode is notified. */
	UFUNCTION(Server, Reliable)
	void DecrementLives();

protected:

	/** This player's current number of remaining lives. This is initialized and managed by the game mode. */
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, ReplicatedUsing = OnRep_CurrentLives, Category = "Game Data")
	uint8 CurrentLives;

	/** OnRep for CurrentLives. Updates client-side information (primarily the user interface). */
	UFUNCTION()
	void OnRep_CurrentLives(uint8 OldValue);



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

// Attribute sets. These have to be created in the same class as their ASC.
protected:

	/** This player's health attribute set. */
	UPROPERTY()
	TObjectPtr<UHealthAttributeSet> HealthSet;

// Event callbacks.
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
