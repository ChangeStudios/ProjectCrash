// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "ModularPlayerState.h"
#include "Characters/Data/PawnData.h"
#include "CrashPlayerState.generated.h"

class UCrashAbilitySystemComponent;
class UCrashGameModeData;
class UPawnData;

/**
 * Base modular player state for this project. Handles core functionality used regardless of game mode: manages pawn
 * data, an ability system component, the team framework, and runtime player statistics.
 */
UCLASS()
class PROJECTCRASH_API ACrashPlayerState : public AModularPlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	ACrashPlayerState(const FObjectInitializer& ObjectInitializer);



	// Initialization.

public:

	/**
	 * Initializes ASC actor info.
	 *
	 * Starts listening for the game mode to finish loading so the pawn data can be initialized.
	 */
	virtual void PostInitializeComponents() override;

	/** Initializes pawn data using the game mode. */
	void OnGameModeLoaded(const UCrashGameModeData* GameModeData);



	// Pawn data.

public:

	/** Sets this player's current pawn data. Should only be called on the server. */
	void SetPawnData(const UPawnData* InPawnData);

	/** Templated getter for retrieving current pawn data. */
	template <class T>
	const T* GetPawnData() const { return Cast<T>(PawnData); }

protected:

	/** Data defining which pawn will be used by this player. Used by the game mode when deciding which pawn class to
	 * spawn for this player. Used by pawns in their initialization; e.g. defines input configurations. */
	UPROPERTY(ReplicatedUsing = OnRep_PawnData)
	TObjectPtr<const UPawnData> PawnData;

	/** OnRep for this player's pawn data. Used for debugging purposes. */
	UFUNCTION()
	void OnRep_PawnData();



	// Ability system.

public:

	/** Game framework component extension event fired when this player receives their pawn's default ability sets.
	 * Used to handle abilities' dependencies on other abilities. */
	static const FName NAME_AbilitiesReady;

// Ability system component.
public:

	/** Typed getter for this player's ASC. */
	UFUNCTION(BlueprintCallable, Category = "Crash|PlayerState")
	UCrashAbilitySystemComponent* GetCrashAbilitySystemComponent() const { return AbilitySystemComponent; }

	/** Interfaced getter for this player's ASC. */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

private:

	/** This player's ability system component. */
	UPROPERTY(VisibleAnywhere, Category = "Crash|PlayerState")
	TObjectPtr<UCrashAbilitySystemComponent> AbilitySystemComponent;
};
