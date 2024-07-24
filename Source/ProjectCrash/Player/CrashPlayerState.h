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
struct FCrashAbilitySet_GrantedHandles;

/**
 * Defines how a client is connected.
 */
UENUM()
enum class EPlayerConnectionType : uint8
{
	// An active player.
	Player = 0,
	// A deactivated player (disconnected).
	InactivePlayer,
	// A spectator connected to a running game.
	Spectator
};



/**
 * Base modular player state for this project. Manages pawn data, an ability system component, the team framework,
 * and runtime player statistics.
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

	/** Initializes ASC actor info. Starts listening for the game mode to finish loading so the pawn data can be
	 * initialized. */
	virtual void PostInitializeComponents() override;

	/** Initializes pawn data using the game mode. */
	void OnGameModeLoaded(const UCrashGameModeData* GameModeData);



	// Player connections.

public:

	/** Changes this player's connection status to "Inactive." Destroys this player state unless the game mode wants to
	 * keep inactive players (defined by game mode data). */
	virtual void OnDeactivated() override;

	/** Changes this player's connection status to "Active." */
	virtual void OnReactivated() override;

	/** Updates this player's current connection type. */
	void SetPlayerConnectionType(EPlayerConnectionType NewConnectionType);

	/** This player's current connection type. */
	EPlayerConnectionType GetPlayerConnectionType() const { return ConnectionType; }

private:

	/** How this player is currently connected to the server (as a spectator, an inactive player, etc.). */
	UPROPERTY(Replicated)
	EPlayerConnectionType ConnectionType;



	// Pawn data.

public:

	/** Sets this player's current pawn data. Should only be called on the server. */
	void SetPawnData(const UPawnData* InPawnData);

	/** Updates this player's current pawn data, destroying their current pawn if necessary, and restarts them. Used
	 * for changing pawns during gameplay via a "Switch Character" menu. For initializing pawn data, use SetPawnData
	 * instead. */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Crash|PlayerState")
	void Server_ChangePawn(const UPawnData* InPawnData);

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

	/** Typed getter for this player's ASC. */
	UFUNCTION(BlueprintCallable, Category = "Crash|PlayerState")
	UCrashAbilitySystemComponent* GetCrashAbilitySystemComponent() const { return AbilitySystemComponent; }

	/** Interfaced getter for this player's ASC. */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

private:

	/** This player's ability system component. */
	UPROPERTY(VisibleAnywhere, Category = "Crash|PlayerState")
	TObjectPtr<UCrashAbilitySystemComponent> AbilitySystemComponent;

	/** Handles to the ability sets granted by our current pawn data. Used to remove these ability sets if our pawn
	 * data changes. */
	TArray<FCrashAbilitySet_GrantedHandles> GrantedPawnDataAbilitySets;
};
