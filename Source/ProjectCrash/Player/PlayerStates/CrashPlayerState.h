// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Teams/CrashTeams.h"
#include "CrashPlayerState.generated.h"

class UChallengerData;
class UHealthAttributeSet;

/** Broadcast when a player's current lives changes. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FLivesChangedSignature, ACrashPlayerState*, Player, uint8, OldLives, uint8, NewLives);

/**
 * The player state used during gameplay (as opposed to menus, lobbies, etc.).
 */
UCLASS()
class PROJECTCRASH_API ACrashPlayerState : public APlayerState, public IAbilitySystemInterface, public ICrashTeamMemberInterface
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



	// Match logic.

// End match.
public:

	UFUNCTION(Client, Reliable)
	void Client_HandleMatchEnded(bool bWon);



	// Team.

// Accessors.
public:

	/** Server-only setter for this player's Team ID. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Player|Player State|Team")
	void SetTeamID(FCrashTeamID InTeamID);

	/** Retrieves this player's current team through the CrashTeamMember interface. */
	virtual FCrashTeamID GetTeamID() const override { return TeamID; }

	/** Blueprint-exposed getter for TeamID. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Player|Player State|Team", Meta = (ToolTip = "This player's current team ID."))
	uint8 K2_GetTeamID() const { return TeamID; }

// Internals.
protected:

	/** This player's current team ID. */
	UPROPERTY(ReplicatedUsing = OnRep_TeamID)
	FCrashTeamID TeamID;

	/** OnRep for Team. */
	UFUNCTION()
	void OnRep_TeamID(FCrashTeamID OldTeamID);



	// Lives.

public:

	/** Getter for CurrentLives. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Player|Player State|Lives", Meta = (ToolTip = "This player's current number of remaining lives."))
	uint8 GetCurrentLives() const { return CurrentLives; }

	/** Decrements this player's current number of lives. If the player runs out of lives, the game mode is notified. */
	UFUNCTION(Server, Reliable)
	void DecrementLives();

	/** Broadcast when this player's number of remaining lives changes. */
	UPROPERTY(BlueprintAssignable)
	FLivesChangedSignature LivesChangedDelegate;

protected:

	/** This player's current number of remaining lives. This is initialized and managed by the game mode. */
	UPROPERTY(ReplicatedUsing = OnRep_CurrentLives)
	uint8 CurrentLives;

	/** OnRep for CurrentLives. Updates client-side information (primarily the user interface). */
	UFUNCTION()
	void OnRep_CurrentLives(uint8 OldValue);



	// Challenger data.

public:

	/** Accessor for CurrentChallenger. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Player|Player State|Challenger Data", Meta = (ToolTip = "The challenger currently selected by this player."))
	UChallengerData* GetCurrentChallenger() const { return CurrentChallenger; }

	/** Server-only setter for CurrentChallenger. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Player|Player State|Challenger Data")
	void UpdateCurrentChallenger(UChallengerData* InChallengerData);

protected:

	/** The challenger currently selected by this player. */
	UPROPERTY(Replicated)
	TObjectPtr<UChallengerData> CurrentChallenger;



	// Ability system.

// Accessors.
public:

	/** ASC interface's default getter. */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/** Typed getter for this player's ASC. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Player|Player State|Ability System")
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
