// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonPlayerController.h"
#include "GameFramework/Teams/CrashTeamAgentInterface.h"
#include "CrashPlayerController.generated.h"

class UCrashAbilitySystemComponent;
class ACrashPlayerState;

/** Fired when this controller's player state is set. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayerStateChangedSignature, const APlayerState*, NewPlayerState);

/**
 * Base modular player controller for this project.
 *
 * Provides access to the player's team via the player state.
 */
UCLASS()
class PROJECTCRASH_API ACrashPlayerController : public ACommonPlayerController, public ICrashTeamAgentInterface
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	ACrashPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());



	// Input.

public:

	/** Instructs this player's ASC to process ability input. */
	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;



	// Events.

public:

	/** Triggers OnPlayerStateChanged. */
	virtual void InitPlayerState() override;

	/** Triggers OnPlayerStateChanged. */
	virtual void CleanupPlayerState() override;

	/** Triggers OnPlayerStateChanged. */
	virtual void OnRep_PlayerState() override;

	/** Fired when this controller's player state is set. */
	UPROPERTY(BlueprintAssignable)
	FPlayerStateChangedSignature PlayerStateChangedDelegate;

protected:

	/** Called when the player state is set or cleared. Default implementation broadcasts TeamChangedDelegate, since
	 * the player state determines this player controller's team. */
	virtual void OnPlayerStateChanged();



	// Teams.

public:

	/** This controller's team cannot be set directly. Its team is driven by the player state. */
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamId) override;

	/** Retrieves this player's team via its player state. Returns NoTeam if this controller does not yet have a player
	 * state. */
	virtual FGenericTeamId GetGenericTeamId() const override;

	/** Returns the delegate fired when this player's team is assigned or changes. */
	virtual FTeamIdChangedSignature* GetTeamIdChangedDelegate() override { return &TeamChangedDelegate; }

private:

	/** Fired when this player changes teams, either because this controller changed player states or because its
	 * player state changed teams. */
	UPROPERTY()
	FTeamIdChangedSignature TeamChangedDelegate;

	/** The last player state to which this controller bound its team. Used to unbind this controller's team from its
	 * old player state when the player state changes. */
	UPROPERTY()
	TObjectPtr<APlayerState> LastSeenPlayerState;

	/** Broadcasts TeamChangedDelegate when this controller's player state changes teams. */
	UFUNCTION()
	void OnPlayerStateChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);



	// Spectating.

public:

	// Tries to spawn at a spectator player start.
	virtual ASpectatorPawn* SpawnSpectatorPawn() override;

	// TODO: Implement.
	virtual void BeginSpectatingState() override;
	virtual void EndSpectatingState() override;
	virtual void BeginPlayingState() override;



	// Utils.

public:

	/** Retrieves this player controller's typed player state. */
	UFUNCTION(BlueprintCallable, Category = "Crash|Player Controller")
	ACrashPlayerState* GetCrashPlayerState() const;

	/** Retrieves this player's ASC, assuming it's stored on their player state. */
	UFUNCTION(BlueprintCallable, Category = "Crash|Player Controller")
	UCrashAbilitySystemComponent* GetCrashAbilitySystemComponent() const;
};
