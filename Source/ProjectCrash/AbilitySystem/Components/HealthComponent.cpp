// Copyright Samuel Reitich 2024.


#include "AbilitySystem/Components/HealthComponent.h"

#include "AbilitySystemLog.h"
#include "CrashAbilitySystemComponent.h"
#include "AbilitySystem/AttributeSets/HealthAttributeBaseValues.h"
#include "AbilitySystem/AttributeSets/HealthAttributeSet.h"
#include "GameFramework/GameModes/CrashGameMode.h"
#include "Kismet/GameplayStatics.h"

UHealthComponent::UHealthComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);

	AbilitySystemComponent = nullptr;
	HealthSet = nullptr;
}

void UHealthComponent::InitializeWithAbilitySystem(UCrashAbilitySystemComponent* InASC, UHealthAttributeBaseValues* InAttributeBaseValues)
{
	AActor* Owner = GetOwner();
	check(Owner);

	if (AbilitySystemComponent)
	{
		ABILITY_LOG(Error, TEXT("Health component for owner [%s] has already been initialized with an ability system."), *GetNameSafe(Owner));
		return;
	}

	AbilitySystemComponent = InASC;
	if (!AbilitySystemComponent)
	{
		ABILITY_LOG(Error, TEXT("Cannot initialize health component for owner [%s] with NULL ability system."), *GetNameSafe(Owner));
		return;
	}

	HealthSet = AbilitySystemComponent->GetSet<UHealthAttributeSet>();
	if (!HealthSet)
	{
		ABILITY_LOG(Error, TEXT("Cannot initialize health component for owner [%s] with NULL health set on the ability system."), *GetNameSafe(Owner));
		return;
	}

	// Bind delegates to the health attribute set's attribute changes and events.
	HealthSet->HealthChangedDelegate.AddDynamic(this, &ThisClass::OnHealthChanged);
	HealthSet->MaxHealthChangedDelegate.AddDynamic(this, &ThisClass::OnMaxHealthChanged);
	HealthSet->OutOfHealthDelegate.AddUObject(this, &ThisClass::OnOutOfHealth);

	// Initialize the attribute set's base values.
	if (IsValid(InAttributeBaseValues))
	{
		AttributeBaseValues = InAttributeBaseValues;

		AbilitySystemComponent->SetNumericAttributeBase(UHealthAttributeSet::GetMaxHealthAttribute(), AttributeBaseValues->BaseMaxHealth);
		AbilitySystemComponent->SetNumericAttributeBase(UHealthAttributeSet::GetHealthAttribute(), AttributeBaseValues->BaseHealth);
		AbilitySystemComponent->SetNumericAttributeBase(UHealthAttributeSet::GetDamageAttribute(), AttributeBaseValues->BaseDamage);
		AbilitySystemComponent->SetNumericAttributeBase(UHealthAttributeSet::GetHealingAttribute(), AttributeBaseValues->BaseHealing);
	}
	else
	{
		ABILITY_LOG(Warning, TEXT("Health component owned by [%s] was not given a valid HealthAttributeBaseValues data asset to initialize its attributes."), *GetNameSafe(Owner));
	}

	// Broadcast the initial changes to the attributes.
	HealthChangedDelegate.Broadcast(this, HealthSet->GetHealth(), HealthSet->GetHealth(), nullptr);
	MaxHealthChangedDelegate.Broadcast( this, HealthSet->GetHealth(), HealthSet->GetHealth(), nullptr);
}

void UHealthComponent::UninitializeFromAbilitySystem()
{
	if (HealthSet)
	{
		HealthSet->HealthChangedDelegate.RemoveAll(this);
		HealthSet->MaxHealthChangedDelegate.RemoveAll(this);
		HealthSet->OutOfHealthDelegate.RemoveAll(this);
	}

	HealthSet = nullptr;
	AbilitySystemComponent = nullptr;
}

void UHealthComponent::OnUnregister()
{
	UninitializeFromAbilitySystem();

	Super::OnUnregister();
}

float UHealthComponent::GetHealth() const
{
	return (HealthSet ? HealthSet->GetHealth() : 0.0f);
}

float UHealthComponent::GetMaxHealth() const
{
	return (HealthSet ? HealthSet->GetMaxHealth() : 0.0f);
}

float UHealthComponent::GetHealthNormalized() const
{
	if (HealthSet)
	{
		const float Health = HealthSet->GetHealth();
		const float MaxHealth = HealthSet->GetMaxHealth();

		return ((MaxHealth > 0.0f) ? (Health / MaxHealth) : 0.0f);
	}

	return 0.0f;
}

void UHealthComponent::OnHealthChanged(AActor* EffectInstigator, AActor* EffectCauser, const FGameplayEffectSpec& EffectSpec, float OldValue, float NewValue)
{
	HealthChangedDelegate.Broadcast(this, OldValue, NewValue, EffectInstigator);
}

void UHealthComponent::OnMaxHealthChanged(AActor* EffectInstigator, AActor* EffectCauser, const FGameplayEffectSpec& EffectSpec, float OldValue, float NewValue)
{
	MaxHealthChangedDelegate.Broadcast(this, OldValue, NewValue, EffectInstigator);
}

void UHealthComponent::OnOutOfHealth(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec& DamageEffectSpec, float DamageMagnitude)
{
#if WITH_SERVER_CODE
	if (AbilitySystemComponent)
	{
		// /* Send the "Event.Ability.Generic.Death" gameplay event through the owner's ability system. This is used to
		//  * trigger a death gameplay ability. */
		// FGameplayEventData Payload;
		// Payload.EventTag = CrashGameplayTags::TAG_Event_Ability_Generic_Death;
		// Payload.Instigator = DamageInstigator;
		// Payload.Target = AbilitySystemComponent->GetAvatarActor();
		// Payload.OptionalObject = DamageEffectSpec.Def;
		// Payload.ContextHandle = DamageEffectSpec.GetEffectContext();
		// Payload.InstigatorTags = *DamageEffectSpec.CapturedSourceTags.GetAggregatedTags();
		// Payload.TargetTags = *DamageEffectSpec.CapturedTargetTags.GetAggregatedTags();
		// Payload.EventMagnitude = DamageMagnitude;
		//
		// // Create a new prediction scope for dying.
		// FScopedPredictionWindow NewScopedWindow(AbilitySystemComponent, true);
		//
		// // Send the event to the owner's ASC.
		// AbilitySystemComponent->HandleGameplayEvent(Payload.EventTag, &Payload);

		// Notify the gamemode that the ASC's avatar has died. Death logic will work with an avatar of any actor class.
		ACrashGameMode* CrashGM = Cast<ACrashGameMode>(UGameplayStatics::GetGameMode(GetWorld()));

		if (CrashGM)
		{
			CrashGM->StartDeath(AbilitySystemComponent->GetAvatarActor());
		}
	}
	else
	{
		ABILITY_LOG(Warning, TEXT("UHealthComponent: Actor [%s] ran out of health, but could not die."), *GetNameSafe(GetOwner()));
	}
#endif // #if WITH_SERVER_CODE
}
