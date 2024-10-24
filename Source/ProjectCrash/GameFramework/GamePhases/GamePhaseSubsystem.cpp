// Copyright Samuel Reitich. All rights reserved.

#include "GameFramework/GamePhases/GamePhaseSubsystem.h"

UGamePhaseSubsystem::UGamePhaseSubsystem()
{
}

void UGamePhaseSubsystem::PostInitialize()
{
	Super::PostInitialize();
}

bool UGamePhaseSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return Super::ShouldCreateSubsystem(Outer);
}

bool UGamePhaseSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return Super::DoesSupportWorldType(WorldType);
}

void UGamePhaseSubsystem::StartPhase(TSubclassOf<UGamePhaseAbility> UGamePhaseAbility, FGamePhaseSignature PhaseEndedCallback)
{
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

void UGamePhaseSubsystem::K2_StartPhase(TSubclassOf<UGamePhaseAbility> Phase, const FGamePhaseDynamicSignature& PhaseEnded)
{
}

void UGamePhaseSubsystem::K2_WhenPhaseStartsOrIsActive(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, FGamePhaseTagDynamicSignature WhenPhaseActive)
{
}

void UGamePhaseSubsystem::K2_WhenPhaseEnds(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, FGamePhaseTagDynamicSignature WhenPhaseEnds)
{
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
