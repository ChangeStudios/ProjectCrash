// Copyright Samuel Reitich 2024.


#include "Player/CrashPlayerState.h"

#include "AbilitySystemGlobals.h"
#include "AbilitySystem/CrashGameplayTags.h"
#include "AbilitySystem/Abilities/CrashAbilitySet.h"
#include "AbilitySystem/AttributeSets/HealthAttributeSet.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"

ACrashPlayerState::ACrashPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Create the ability system component.
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UCrashAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	NetUpdateFrequency = 100.0f; // The ASC Needs to be updated at a high frequency.

	// Create this player's attribute sets. These must be created in the same class as their ASC.
	HealthSet = CreateDefaultSubobject<UHealthAttributeSet>(TEXT("HealthSet"));
}

void ACrashPlayerState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Initialize the ASC's actor info with this player state as its owner.
	check(AbilitySystemComponent);
	AbilitySystemComponent->InitAbilityActorInfo(this, GetPawn());

	// Grant the player's default ability set.
	if (PlayerAbilitySet)
	{
		PlayerAbilitySet->GiveToAbilitySystem(AbilitySystemComponent, nullptr, this);
	}

	/* Bind the OnInputBlockingChanged callback to when this player's ASC gains or loses the InputBlocking tag. */
	InputBlockingDelegate = AbilitySystemComponent->RegisterGameplayTagEvent(CrashGameplayTags::TAG_Ability_Behavior_InputBlocking, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ACrashPlayerState::OnInputBlockingChanged);
}

UAbilitySystemComponent* ACrashPlayerState::GetAbilitySystemComponent() const
{
	// The interfaced accessor will always return the typed ASC.
	return GetCrashAbilitySystemComponent();
}

void ACrashPlayerState::OnInputBlockingChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (GetPawn() && GetPawn()->IsLocallyControlled())
	{
		if (NewCount > 0)
		{
			GetPlayerController()->ClientIgnoreLookInput(true);
		}
		else
		{
			GetPlayerController()->ClientIgnoreLookInput(false);
		}
	}
}
