// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "ModularPlayerState.h"
#include "AbilitySystem/Abilities/CrashAbilitySet.h"
#include "Characters/Data/PawnData.h"
#include "GameFramework/Teams/CrashTeamAgentInterface.h"
#include "CrashPlayerState.generated.h"

class UInventoryComponent;
class UCrashAbilitySystemComponent;
class UCrashGameModeData;
class UPawnData;

/** Fired when this player's current pawn data changes, as a result of them changing their current playable
 * character. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPawnDataChangedSignature, const UPawnData*, OldPawnData, const UPawnData*, NewPawnData);

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
class PROJECTCRASH_API ACrashPlayerState : public AModularPlayerState, public IAbilitySystemInterface, public ICrashTeamAgentInterface
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
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Crash|Player State")
	void Server_ChangePawn(const UPawnData* InPawnData);

	/** Templated getter for retrieving current pawn data. */
	template <class T>
	const T* GetPawnData() const { return Cast<T>(PawnData); }

	/** Fired on server and clients when this player's selected pawn data changes, as a result of changing their
	 * selected playable character. This is not always fired when changing pawns; e.g. temporarily taking control of a
	 * "drone" pawn will not trigger this. */
	UPROPERTY(BlueprintAssignable)
	FPawnDataChangedSignature PawnDataChangedDelegate;

protected:

	/** Data defining which pawn will be used by this player. Used by the game mode when deciding which pawn class to
	 * spawn for this player. Used by pawns in their initialization; e.g. defines input configurations. This is not
	 * updated to reflect players temporarily switching pawns, e.g. taking control of a "drone" pawn. */
	UPROPERTY(ReplicatedUsing = OnRep_PawnData)
	TObjectPtr<const UPawnData> PawnData;

	/** OnRep for this player's pawn data. Used for debugging purposes. */
	UFUNCTION()
	void OnRep_PawnData(const UPawnData* OldPawnData);



	// Ability system.

public:

	/** Typed getter for this player's ASC. */
	UFUNCTION(BlueprintCallable, Category = "Crash|Player State")
	UCrashAbilitySystemComponent* GetCrashAbilitySystemComponent() const { return AbilitySystemComponent; }

	/** Interfaced getter for this player's ASC. */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

private:

	/** This player's ability system component. */
	UPROPERTY(VisibleAnywhere, Category = "Crash|Player State")
	TObjectPtr<UCrashAbilitySystemComponent> AbilitySystemComponent;



	// Teams.

public:

	/** Sets this player's current team ID. Can only be called on the server. */
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamId) override;

	/** Returns this player's current team ID. */
	virtual FGenericTeamId GetGenericTeamId() const override { return TeamId; }

	/** Returns the delegate fired when this player's team ID changes. */
	virtual FTeamIdChangedSignature* GetTeamIdChangedDelegate() override { return &OnTeamIdChangedDelegate; }

	/** Blueprint-exposed wrapper for GetGenericTeamId that also converts the ID to an integer (NoTeam ->
	 * INDEX_NONE). */
	UFUNCTION(BlueprintPure, Meta = (ToolTip = "The ID of the team to which this player currently belongs."))
	int32 GetTeamId() const { return GenericTeamIdToInteger(GetGenericTeamId()); }

private:

	/** Delegate fired when this player's team ID changes. */
	UPROPERTY()
	FTeamIdChangedSignature OnTeamIdChangedDelegate;

	/** The ID of the team to which this player currently belongs. */
	UPROPERTY(ReplicatedUsing = OnRep_TeamId)
	FGenericTeamId TeamId;

	/** Broadcasts OnTeamIdChangedDelegate when this player's team ID changes. */
	UFUNCTION()
	void OnRep_TeamId(FGenericTeamId OldTeamId);
};
