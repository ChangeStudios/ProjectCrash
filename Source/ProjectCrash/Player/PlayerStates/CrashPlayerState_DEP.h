// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "CrashPlayerStateBase.h"
#include "GameFramework/Teams/CrashTeams.h"
#include "CrashPlayerState_DEP.generated.h"

class UChallengerSkinData;
class UChallengerData;
class UHealthAttributeSet;

/** Broadcast when a player's current lives changes. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FLivesChangedSignature, ACrashPlayerState_DEP*, Player, uint8, OldLives, uint8, NewLives);

/**
 * The primary player state used during gameplay. Contains the player's ability system component.
 */
UCLASS()
class PROJECTCRASH_API ACrashPlayerState_DEP : public ACrashPlayerStateBase, public IAbilitySystemInterface, public ICrashTeamMemberInterface
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	ACrashPlayerState_DEP(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());



	// Match logic.

// End match.
public:

	/** Displays a match result pop-up when the match ends. */
	UFUNCTION(Client, Reliable)
	void Client_HandleMatchEnded(bool bWon);

	/** Returns the player to the menu when the post-match phase ends. */
	UFUNCTION(Client, Reliable)
	void Client_HandleLeavingMap();

	/** Timer used to return to the menu after the post-match sequence (i.e. where the "victory"/"defeat" screen is
	 * displayed). */
	FTimerHandle EndMatchTimer;



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
