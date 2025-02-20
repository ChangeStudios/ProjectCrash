// Copyright Samuel Reitich. All rights reserved.

#include "GameFramework/GamePhases/GamePhaseSubsystem.h"

#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "GameFramework/CrashLogging.h"
#include "GameFramework/GameStateBase.h"
#include "GamePhaseAbility.h"
#include "GameFramework/CheatManager.h"

UGamePhaseSubsystem::UGamePhaseSubsystem()
{
}

void UGamePhaseSubsystem::StartMatch()
{
	const UWorld* World = GetWorld();
	UCrashAbilitySystemComponent* GameStateASC = World->GetGameState()->FindComponentByClass<UCrashAbilitySystemComponent>();

	TArray<FGameplayAbilitySpec*> ActivePhaseSpecs;
	for (const auto& KVP : ActivePhases)
	{

		const FGameplayAbilitySpecHandle ActivePhaseHandle = KVP.Key;
		if (FGameplayAbilitySpec* Spec = GameStateASC->FindAbilitySpecFromHandle(ActivePhaseHandle))
		{
			ActivePhaseSpecs.Add(Spec);
		}
	}

	for (const FGameplayAbilitySpec* ActivePhaseSpec : ActivePhaseSpecs)
	{
		UGamePhaseAbility* ActivePhaseAbility = CastChecked<UGamePhaseAbility>(ActivePhaseSpec->GetPrimaryInstance());
		ActivePhaseAbility->OnMatchStarted();
	}
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
		// Phase failed to start. This may happen before the ASC has been initialized and can be ignored.
		else
		{
			PhaseEndedCallback.ExecuteIfBound(nullptr);
		}
	}
}

void UGamePhaseSubsystem::WhenPhaseStartsOrIsActive(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, const FGamePhaseTagSignature& WhenPhaseActive)
{
	/** Register the given callback to when a matching phase starts. */
	FPhaseListener Listener;
	Listener.PhaseTag = PhaseTag;
	Listener.MatchType = MatchType;
	Listener.PhaseCallback = WhenPhaseActive;
	PhaseStartListeners.Add(Listener);

	/** Immediately fire the callback if the phase is already active. */
	if (IsPhaseActive(PhaseTag))
	{
		WhenPhaseActive.ExecuteIfBound(PhaseTag);
	}
}

void UGamePhaseSubsystem::WhenPhaseEnds(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, const FGamePhaseTagSignature& WhenPhaseEnds)
{
	/** Register the given callback to when a matching phase ends. */
	FPhaseListener Listener;
	Listener.PhaseTag = PhaseTag;
	Listener.MatchType = MatchType;
	Listener.PhaseCallback = WhenPhaseEnds;
	PhaseEndListeners.Add(Listener);
}

bool UGamePhaseSubsystem::IsPhaseActive(const FGameplayTag& PhaseTag) const
{
	for (const auto& KVP : ActivePhases)
	{
		const FGamePhaseEntry& PhaseEntry = KVP.Value;
		if (PhaseEntry.PhaseTag.MatchesTag(PhaseTag))
		{
			return true;
		}
	}

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
	const FGameplayTag NewPhaseTag = PhaseAbility->GetGamePhaseTag();

	UE_LOG(LogGamePhase, Log, TEXT("Beginning Phase [%s] (%s)..."), *NewPhaseTag.ToString(), *GetNameSafe(PhaseAbility));

	const UWorld* World = GetWorld();
	UCrashAbilitySystemComponent* GameStateASC = World->GetGameState()->FindComponentByClass<UCrashAbilitySystemComponent>();
	if (ensure(GameStateASC))
	{
		// Collect every active phase's gameplay ability.
		TArray<FGameplayAbilitySpec*> ActivePhaseSpecs;
		for (const auto& KVP : ActivePhases)
		{
			const FGameplayAbilitySpecHandle ActivePhaseHandle = KVP.Key;
			if (FGameplayAbilitySpec* Spec = GameStateASC->FindAbilitySpecFromHandle(ActivePhaseHandle))
			{
				ActivePhaseSpecs.Add(Spec);
			}
		}

		for (const FGameplayAbilitySpec* ActivePhaseSpec : ActivePhaseSpecs)
		{
			const UGamePhaseAbility* ActivePhaseAbility = CastChecked<UGamePhaseAbility>(ActivePhaseSpec->Ability);
			const FGameplayTag ActivePhaseTag = ActivePhaseAbility->GetGamePhaseTag();

			// End any phases that are not parents of the new phase.
			if (!NewPhaseTag.MatchesTag(ActivePhaseTag))
			{
				// End phases by cancelling the abilities.
				FGameplayAbilitySpecHandle HandleToEnd = ActivePhaseSpec->Handle;
				GameStateASC->CancelAbilitiesByFunc([HandleToEnd](const UCrashGameplayAbilityBase* CrashAbility, FGameplayAbilitySpecHandle Handle)
				{
					return Handle == HandleToEnd;
				}, true);
			}
		}

		FGamePhaseEntry& Entry = ActivePhases.FindOrAdd(PhaseAbilityHandle);
		Entry.PhaseTag = NewPhaseTag;

		// Notify listeners that this phase started.
		for (const FPhaseListener& Listener : PhaseStartListeners)
		{
			if (Listener.IsMatch(NewPhaseTag))
			{
				Listener.PhaseCallback.ExecuteIfBound(NewPhaseTag);
			}
		}
	}
}

void UGamePhaseSubsystem::OnEndPhase(const UGamePhaseAbility* PhaseAbility, const FGameplayAbilitySpecHandle PhaseAbilityHandle)
{
	const FGameplayTag EndedPhaseTag = PhaseAbility->GetGamePhaseTag();

	UE_LOG(LogGamePhase, Log, TEXT("\t\t...Ended phase [%s] (%s)."), *EndedPhaseTag.ToString(), *GetNameSafe(PhaseAbility));

	// Clear the active phase entry.
	const FGamePhaseEntry& Entry = ActivePhases.FindChecked(PhaseAbilityHandle);
	Entry.PhaseEndedCallback.ExecuteIfBound(PhaseAbility);
	ActivePhases.Remove(PhaseAbilityHandle);

	// Notify listeners that this phase started.
	for (const FPhaseListener& Listener : PhaseStartListeners)
	{
		if (Listener.IsMatch(EndedPhaseTag))
		{
			Listener.PhaseCallback.ExecuteIfBound(EndedPhaseTag);
		}
	}
}

bool UGamePhaseSubsystem::FPhaseListener::IsMatch(const FGameplayTag& OtherPhaseTag) const
{
	switch(MatchType)
	{
		case EPhaseTagMatchType::ExactMatch:
			return OtherPhaseTag.MatchesTagExact(PhaseTag);
		case EPhaseTagMatchType::PartialMatch:
			return OtherPhaseTag.MatchesTag(PhaseTag);
	}

	return false;
}
