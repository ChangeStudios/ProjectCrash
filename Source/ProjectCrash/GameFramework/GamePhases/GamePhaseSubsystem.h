// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "Subsystems/WorldSubsystem.h"
#include "GamePhaseSubsystem.generated.h"

class UGamePhaseAbility;

DECLARE_DELEGATE_OneParam(FGamePhaseSignature, const UGamePhaseAbility* Phase);
DECLARE_DYNAMIC_DELEGATE_OneParam(FGamePhaseDynamicSignature, const UGamePhaseAbility*, Phase);

DECLARE_DELEGATE_OneParam(FGamePhaseTagSignature, const FGameplayTag& PhaseTag);
DECLARE_DYNAMIC_DELEGATE_OneParam(FGamePhaseTagDynamicSignature, const FGameplayTag&, PhaseTag);

UENUM(BlueprintType)
enum class EPhaseTagMatchType : uint8
{
	ExactMatch,
	PartialMatch
};

/**
 * 
 */
UCLASS()
class PROJECTCRASH_API UGamePhaseSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

	friend class UGamePhaseAbility;

	// Initialization.

public:

	UGamePhaseSubsystem();

	virtual void PostInitialize() override;

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

protected:

	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;



	// Phase transitions.

public:

	void StartPhase(TSubclassOf<UGamePhaseAbility> UGamePhaseAbility, FGamePhaseSignature PhaseEndedCallback = FGamePhaseSignature());

	void WhenPhaseStartsOrIsActive(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, const FGamePhaseTagSignature& WhenPhaseActive);

	void WhenPhaseEnds(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, const FGamePhaseTagSignature& WhenPhaseEnds);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, BlueprintPure = "false", meta = (AutoCreateRefTerm = "PhaseTag"))
	bool IsPhaseActive(const FGameplayTag& PhaseTag) const;

protected:

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Game Phase", DisplayName = "Start Phase", meta = (AutoCreateRefTerm = "PhaseEnded"))
	void K2_StartPhase(TSubclassOf<UGamePhaseAbility> Phase, const FGamePhaseDynamicSignature& PhaseEnded);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Game Phase", DisplayName = "When Phase Starts or Is Active", meta = (AutoCreateRefTerm = "WhenPhaseActive"))
	void K2_WhenPhaseStartsOrIsActive(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, FGamePhaseTagDynamicSignature WhenPhaseActive);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Game Phase", DisplayName = "When Phase Ends", meta = (AutoCreateRefTerm = "WhenPhaseEnds"))
	void K2_WhenPhaseEnds(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, FGamePhaseTagDynamicSignature WhenPhaseEnds);

	void OnBeginPhase(const UGamePhaseAbility* PhaseAbility, const FGameplayAbilitySpecHandle PhaseAbilityHandle);
	void OnEndPhase(const UGamePhaseAbility* PhaseAbility, const FGameplayAbilitySpecHandle PhaseAbilityHandle);



	// Internals.

// Phase tracking.
private:

	struct FGamePhaseEntry
	{
	public:
		FGameplayTag PhaseTag;
		FGamePhaseSignature PhaseEndedCallback;
	};

	TMap<FGameplayAbilitySpecHandle, FGamePhaseEntry> ActivePhases;

// Observers.
private:

	struct FPhaseObserver
	{
	public:
		bool IsMatch(const FGameplayTag& OtherPhaseTag) const;

		FGameplayTag PhaseTag;
		EPhaseTagMatchType MatchType = EPhaseTagMatchType::ExactMatch;
		FGamePhaseTagSignature PhaseCallback;
	};

	TArray<FPhaseObserver> PhaseStartObservers;
	TArray<FPhaseObserver> PhaseEndObservers;
};
