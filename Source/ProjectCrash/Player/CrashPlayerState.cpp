// Copyright Samuel Reitich 2024.


#include "Player/CrashPlayerState.h"

#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"

ACrashPlayerState::ACrashPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Create the ability system component.
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UCrashAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	NetUpdateFrequency = 100.0f; // The ASC Needs to be updated at a high frequency.
}

void ACrashPlayerState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Initialize the ASC's actor info with this player state as its owner.
	check(AbilitySystemComponent);
	AbilitySystemComponent->InitAbilityActorInfo(this, GetPawn());
}

UAbilitySystemComponent* ACrashPlayerState::GetAbilitySystemComponent() const
{
	// The interfaced accessor will always return the typed ASC.
	return GetCrashAbilitySystemComponent();
}
