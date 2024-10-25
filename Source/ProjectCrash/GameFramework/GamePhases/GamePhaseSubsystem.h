// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "Subsystems/WorldSubsystem.h"
#include "GamePhaseSubsystem.generated.h"

class UGamePhaseAbility;

/** Delegate signature for game phase ability callbacks. */
DECLARE_DELEGATE_OneParam(FGamePhaseSignature, const UGamePhaseAbility* Phase);
DECLARE_DYNAMIC_DELEGATE_OneParam(FGamePhaseDynamicSignature, const UGamePhaseAbility*, Phase);

/** Delegate signature for game phase tag callbacks. */
DECLARE_DELEGATE_OneParam(FGamePhaseTagSignature, const FGameplayTag& PhaseTag);
DECLARE_DYNAMIC_DELEGATE_OneParam(FGamePhaseTagDynamicSignature, const FGameplayTag&, PhaseTag);

/** Match rule for game phase tags. */
UENUM(BlueprintType)
enum class EPhaseTagMatchType : uint8
{
	// An exact match will only receive messages with exactly the same channel
	// (e.g., registering for "A.B" will match a broadcast of A.B but not A.B.C)
	ExactMatch,

	// A partial match will receive any messages rooted in the same channel
	// (e.g., registering for "A.B" will match a broadcast of A.B as well as A.B.C)
	PartialMatch
};

/**
 * Manages gameplay flow via "game phases." Active game phases are represented by a Game Phase Ability in the game
 * state. Game phases are identified via tags: PreGame.CharacterSelection, Game.WarmUp, Game.Playing, etc.
 *
 * Parent and child phases can be active at the same time (e.g. Game.Playing and Game.Playing.SuddenDeath), but not
 * sibling phases (e.g. Game.Playing and Game.PostGame). Activating a new phase automatically ends any phases that are
 * not parents of the new phase.
 *
 * @see UGamePhaseAbility
 */
UCLASS()
class PROJECTCRASH_API UGamePhaseSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

	friend class UGamePhaseAbility;

	// Initialization.

public:

	/** Default constructor. */
	UGamePhaseSubsystem();

protected:

	/** Restricts this subsystem to game and PIE worlds. */
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;



	// Phase transitions.

public:

	/** Starts the given phase by activating the ability on the game state. Ends all non-parent phases. */
	void StartPhase(TSubclassOf<UGamePhaseAbility> PhaseAbility, FGamePhaseSignature PhaseEndedCallback = FGamePhaseSignature());

	/** Binds the given delegate to when a phase with the specified tag starts. Immediately fires if the phase is already
	 * active. */
	void WhenPhaseStartsOrIsActive(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, const FGamePhaseTagSignature& WhenPhaseActive);

	/** Binds the given delegate to when a phase with the specified tag ends. */
	void WhenPhaseEnds(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, const FGamePhaseTagSignature& WhenPhaseEnds);

	/** Returns whether a phase with the specified tag is currently active. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, BlueprintPure = "false", meta = (AutoCreateRefTerm = "PhaseTag"))
	bool IsPhaseActive(const FGameplayTag& PhaseTag) const;

protected:

	/** Blueprint wrapper for StartPhase. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Game Phase", DisplayName = "Start Phase", meta = (AutoCreateRefTerm = "PhaseEnded", ToolTip = "Starts the given phase by activating the ability on the game state. Ends all non-parent phases."))
	void K2_StartPhase(TSubclassOf<UGamePhaseAbility> PhaseAbility, const FGamePhaseDynamicSignature& OnPhaseEnded);

	/** Blueprint wrapper for WhenPhaseStartsOrIsActive. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Game Phase", DisplayName = "When Phase Starts or Is Active", meta = (AutoCreateRefTerm = "WhenPhaseActive", ToolTip = "Binds the given delegate to when a phase with the specified tag starts. Immediately fires if the phase is already active."))
	void K2_WhenPhaseStartsOrIsActive(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, FGamePhaseTagDynamicSignature WhenPhaseActive);

	/** Blueprint wrapper for WhenPhaseEnds. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Game Phase", DisplayName = "When Phase Ends", meta = (AutoCreateRefTerm = "WhenPhaseEnds", ToolTip = "Binds the given delegate to when a phase with the specified tag ends."))
	void K2_WhenPhaseEnds(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, FGamePhaseTagDynamicSignature WhenPhaseEnds);

	/** Handles ending non-parent phases and executing callbacks when a new phase begins. */
	void OnBeginPhase(const UGamePhaseAbility* PhaseAbility, const FGameplayAbilitySpecHandle PhaseAbilityHandle);

	/** Fires relevant callbacks when a phase ends. */
	void OnEndPhase(const UGamePhaseAbility* PhaseAbility, const FGameplayAbilitySpecHandle PhaseAbilityHandle);



	// Internals.

// Phase tracking.
private:

	/** Represents an active phase. */
	struct FGamePhaseEntry
	{
		FGameplayTag PhaseTag;
		FGamePhaseSignature PhaseEndedCallback;
	};

	/** Tracks runtime data for active phase abilities. */
	TMap<FGameplayAbilitySpecHandle, FGamePhaseEntry> ActivePhases;

// Phase change listeners.
private:

	/** Stores a callback that should be fired when transitioning to or from a specified phase. */
	struct FPhaseListener
	{
		bool IsMatch(const FGameplayTag& OtherPhaseTag) const;

		FGameplayTag PhaseTag;
		EPhaseTagMatchType MatchType = EPhaseTagMatchType::ExactMatch;
		FGamePhaseTagSignature PhaseCallback;
	};

	/** Callbacks registered to the start of a phase. */
	TArray<FPhaseListener> PhaseStartListeners;

	/** Callbacks registered to the end of a phase. */
	TArray<FPhaseListener> PhaseEndListeners;
};
