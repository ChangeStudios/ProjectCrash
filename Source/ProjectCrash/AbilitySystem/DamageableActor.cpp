// Copyright Samuel Reitich. All rights reserved.

#include "AbilitySystem/DamageableActor.h"

#include "AbilitySystemGlobals.h"
#include "AttributeSets/HealthAttributeSet.h"
#include "Components/CrashAbilitySystemComponent.h"

ADamageableActor::ADamageableActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	// Construct the ASC.
	AbilitySystemComponent = CreateDefaultSubobject<UCrashAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Full);
	MinNetUpdateFrequency = 2.0f;
	NetUpdateFrequency = 20.0f; // Static actors don't need as high of an update frequency.
	HealthSet = CreateDefaultSubobject<UHealthAttributeSet>(TEXT("HealthSet"));
}

void ADamageableActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Initialize ASC.
	check(AbilitySystemComponent);
	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	// Initialize health set and listen for events on the server.
	if (HasAuthority())
	{
		HealthSet->InitMaxHealth(Health);
		HealthSet->InitHealth(Health);

		HealthSet->HealthAttributeChangedDelegate.AddDynamic(this, &ADamageableActor::OnDamageReceived);
		HealthSet->OutOfHealthAttributeDelegate.AddUObject(this, &ADamageableActor::OnDeath);
	}
}

void ADamageableActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Clear delegate bindings.
	if (HasAuthority())
	{
		HealthSet->HealthAttributeChangedDelegate.RemoveAll(this);
		HealthSet->OutOfHealthAttributeDelegate.RemoveAll(this);
	}

	Super::EndPlay(EndPlayReason);
}

UAbilitySystemComponent* ADamageableActor::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ADamageableActor::OnDamageReceived(AActor* EffectInstigator, const FGameplayEffectSpec& EffectSpec, float OldValue, float NewValue)
{
	// Ignore healing.
	if (NewValue >= OldValue)
	{
		return;
	}

	UAbilitySystemComponent* InstigatingASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(EffectInstigator);
	const float Magnitude = (OldValue - NewValue);
	FGameplayEffectContextHandle Context = EffectSpec.GetContext();

	// Trigger multicast to replicate the event to clients.
	Multicast_OnDamageReceived(InstigatingASC, Magnitude, Context);
}

void ADamageableActor::OnDeath(AActor* EffectInstigator, const FGameplayEffectSpec& EffectSpec, float EffectMagnitude)
{	
	UAbilitySystemComponent* InstigatingASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(EffectInstigator);
	FGameplayEffectContextHandle Context = EffectSpec.GetContext();

	// Trigger multicast to replicate the event to clients.
	Multicast_OnDeath(InstigatingASC, EffectMagnitude, Context);
}

void ADamageableActor::Multicast_OnDeath_Implementation(UAbilitySystemComponent* InstigatingASC, float DamageAmount, const FGameplayEffectContextHandle& EffectContext)
{
	// Call BP event locally.
	K2_OnDeath(InstigatingASC, DamageAmount, EffectContext);
}

void ADamageableActor::Multicast_OnDamageReceived_Implementation(UAbilitySystemComponent* InstigatingASC, float DamageAmount, const FGameplayEffectContextHandle& EffectContext)
{
	// Call BP event locally.
	K2_OnDamageReceived(InstigatingASC, DamageAmount, EffectContext);
}

