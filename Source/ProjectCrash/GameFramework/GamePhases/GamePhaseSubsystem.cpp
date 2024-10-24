// Copyright Samuel Reitich. All rights reserved.

#include "GameFramework/GamePhases/GamePhaseSubsystem.h"

#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "GameFramework/CrashLogging.h"
#include "GameFramework/GameStateBase.h"
#include "GamePhaseAbility.h"

UGamePhaseSubsystem::UGamePhaseSubsystem()
{
}

bool UGamePhaseSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return (WorldType == EWorldType::Game || WorldType == EWorldType::PIE);
}

void UGamePhaseSubsystem::StartPhase(TSubclassOf<UGamePhaseAbility> PhaseAbility, FGamePhaseSignature PhaseEndedCallback)
{
	check(PhaseAbility);
	UWorld* World = GetWorld();
	UCrashAbilitySystemComponent* GameStateASC = World->GetGameState()->FindComponentByClass<UCrashAbilitySystemComponent>();
	if (ensure(GameStateASC))
	{
		// Try to activate (start) the new phase.
		FGameplayAbilitySpec PhaseSpec(PhaseAbility, 1, 0, this);
		FGameplayAbilitySpecHandle SpecHandle = GameStateASC->GiveAbilityAndActivateOnce(PhaseSpec);
		FGameplayAbilitySpec* NewPhase = GameStateASC->FindAbilitySpecFromHandle(SpecHandle);

		// Register the active phase if it was successfully started.
		if (NewPhase && NewPhase->IsActive())
		{
			FGamePhaseEntry& Entry = ActivePhases.FindOrAdd(SpecHandle);
			Entry.PhaseEndedCallback = PhaseEndedCallback;
		}
		// Phase failed to start.
		else
		{
			UE_LOG(LogGamePhase, Error, TEXT("Failed to start phase [%s]. Phase ability: [%s]."), *PhaseAbility->GetDefaultObject<UGamePhaseAbility>()->GetGamePhaseTag().ToString(), *GetNameSafe(PhaseAbility));
			PhaseEndedCallback.ExecuteIfBound(nullptr);
		}
	}
}

void UGamePhaseSubsystem::WhenPhaseStartsOrIsActive(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, const FGamePhaseTagSignature& WhenPhaseActive)
{
}

void UGamePhaseSubsystem::WhenPhaseEnds(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, const FGamePhaseTagSignature& WhenPhaseEnds)
{
}

bool UGamePhaseSubsystem::IsPhaseActive(const FGameplayTag& PhaseTag) const
{
	return false;
}

void UGamePhaseSubsystem::K2_StartPhase(TSubclassOf<UGamePhaseAbility> PhaseAbility, const FGamePhaseDynamicSignature& OnPhaseEnded)
{
	if (!ensureAlwaysMsgf(PhaseAbility, TEXT("Failed to start game phase: invalid phase ability given.")))
	{
		return;
	}

	// Create a delegate that fires the BP node's callback pin when the new phase ends.
	const FGamePhaseSignature EndedDelegate = FGamePhaseSignature::CreateWeakLambda(const_cast<UObject*>(OnPhaseEnded.GetUObject()),
		[OnPhaseEnded](const UGamePhaseAbility* PhaseAbility)
		{
			OnPhaseEnded.ExecuteIfBound(PhaseAbility);
		});

	StartPhase(PhaseAbility, EndedDelegate);
}

void UGamePhaseSubsystem::K2_WhenPhaseStartsOrIsActive(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, FGamePhaseTagDynamicSignature WhenPhaseActive)
{
	// Wrap the BP node's callback pin in a delegate that can be registered as an observer.
	const FGamePhaseTagSignature ActiveDelegate = FGamePhaseTagSignature::CreateWeakLambda(WhenPhaseActive.GetUObject(),
		[WhenPhaseActive](const FGameplayTag& PhaseTag)
		{
			WhenPhaseActive.ExecuteIfBound(PhaseTag);
		});

	WhenPhaseStartsOrIsActive(PhaseTag, MatchType, ActiveDelegate);
}

void UGamePhaseSubsystem::K2_WhenPhaseEnds(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, FGamePhaseTagDynamicSignature OnPhaseEnded)
{
	// Wrap the BP node's callback pin in a delegate that can be registered as an observer.
	const FGamePhaseTagSignature EndedDelegate = FGamePhaseTagSignature::CreateWeakLambda(OnPhaseEnded.GetUObject(),
		[OnPhaseEnded](const FGameplayTag& PhaseTag)
		{
			OnPhaseEnded.ExecuteIfBound(PhaseTag);
		});

	WhenPhaseEnds(PhaseTag, MatchType, EndedDelegate);
}

void UGamePhaseSubsystem::OnBeginPhase(const UGamePhaseAbility* PhaseAbility, const FGameplayAbilitySpecHandle PhaseAbilityHandle)
{
}

void UGamePhaseSubsystem::OnEndPhase(const UGamePhaseAbility* PhaseAbility, const FGameplayAbilitySpecHandle PhaseAbilityHandle)
{
}

bool UGamePhaseSubsystem::FPhaseObserver::IsMatch(const FGameplayTag& OtherPhaseTag) const
{
	return false;
}
