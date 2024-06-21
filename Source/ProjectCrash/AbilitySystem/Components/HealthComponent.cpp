// Copyright Samuel Reitich. All rights reserved.


#include "AbilitySystem/Components/HealthComponent.h"

#include "AbilitySystemLog.h"
#include "CrashAbilitySystemComponent.h"
#include "CrashGameplayTags.h"
#include "AbilitySystem/AttributeSets/HealthAttributeBaseValues.h"
#include "AbilitySystem/AttributeSets/HealthAttributeSet.h"
#include "GameFramework/CrashLogging.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/GameModes/Game/CrashGameMode_DEP.h"
#include "GameFramework/GameStates/CrashGameState_DEP.h"
#include "GameFramework/Messages/CrashVerbMessage.h"
#include "GameFramework/Messages/CrashVerbMessageHelpers.h"
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

void UHealthComponent::InitializeWithAbilitySystem(UCrashAbilitySystemComponent* InASC)
{
	AActor* Owner = GetOwner();
	check(Owner);

	if (AbilitySystemComponent)
	{
		ABILITY_LOG(Error, TEXT("Health component for owner [%s] has already been initialized with an ability system owned by: [%s]."), *GetNameSafe(Owner), *GetNameSafe(AbilitySystemComponent->GetOwnerActor()));
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
	HealthSet->HealthAttributeChangedDelegate.AddDynamic(this, &ThisClass::OnHealthChanged);
	HealthSet->MaxHealthAttributeChangedDelegate.AddDynamic(this, &ThisClass::OnMaxHealthChanged);
	HealthSet->OutOfHealthAttributeDelegate.AddUObject(this, &ThisClass::OnOutOfHealth);

	// Initialize the attributes to the attribute set's default values.
	AbilitySystemComponent->SetNumericAttributeBase(UHealthAttributeSet::GetMaxHealthAttribute(), HealthSet->GetMaxHealth());
	AbilitySystemComponent->SetNumericAttributeBase(UHealthAttributeSet::GetHealthAttribute(), HealthSet->GetHealth());
	AbilitySystemComponent->SetNumericAttributeBase(UHealthAttributeSet::GetDamageAttribute(), 0.0f);
	AbilitySystemComponent->SetNumericAttributeBase(UHealthAttributeSet::GetHealingAttribute(), 0.0f);

	// Broadcast the initial changes to the attributes.
	HealthChangedDelegate.Broadcast(this, nullptr, HealthSet->GetHealth(), HealthSet->GetHealth());
	MaxHealthChangedDelegate.Broadcast( this, nullptr, HealthSet->GetMaxHealth(), HealthSet->GetMaxHealth());
}

void UHealthComponent::UninitializeFromAbilitySystem()
{
	if (HealthSet)
	{
		HealthSet->HealthAttributeChangedDelegate.RemoveAll(this);
		HealthSet->MaxHealthAttributeChangedDelegate.RemoveAll(this);
		HealthSet->OutOfHealthAttributeDelegate.RemoveAll(this);
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
	HealthChangedDelegate.Broadcast(this, EffectInstigator, OldValue, NewValue);
}

void UHealthComponent::OnMaxHealthChanged(AActor* EffectInstigator, AActor* EffectCauser, const FGameplayEffectSpec& EffectSpec, float OldValue, float NewValue)
{
	MaxHealthChangedDelegate.Broadcast(this, EffectInstigator, OldValue, NewValue);
}

void UHealthComponent::OnOutOfHealth(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec& DamageEffectSpec, float DamageMagnitude)
{
#if WITH_SERVER_CODE

	if (AbilitySystemComponent)
	{
		// Send a "Death" event through the dying actor's ASC. This can be used to trigger a death ability.
		const FDeathData DeathData = FDeathData
		(
			AbilitySystemComponent->GetAvatarActor(),
			AbilitySystemComponent->AbilityActorInfo->PlayerController.Get(),
			AbilitySystemComponent,
			DamageInstigator,
			DamageCauser,
			DamageEffectSpec,
			DamageMagnitude
		);

		/* Send a standardized verb message that other systems can observe. This is used for things like updating the
		 * kill-feed. */
		if (AbilitySystemComponent && UGameplayMessageSubsystem::HasInstance(GetWorld()))
		{
			FCrashVerbMessage DeathMessage;
			DeathMessage.Verb = CrashGameplayTags::TAG_Message_Death;
			DeathMessage.Instigator = DamageInstigator;
			DeathMessage.InstigatorTags = *DamageEffectSpec.CapturedSourceTags.GetAggregatedTags();
			DeathMessage.Target = UCrashVerbMessageHelpers::GetPlayerStateFromObject(AbilitySystemComponent->GetAvatarActor());
			DeathMessage.TargetTags = *DamageEffectSpec.CapturedTargetTags.GetAggregatedTags();

			// Broadcast the message on the server.
			UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
			MessageSystem.BroadcastMessage(DeathMessage.Verb, DeathMessage);

			// Broadcast the message to clients.
			if (ACrashGameState_DEP* GS = Cast<ACrashGameState_DEP>(UGameplayStatics::GetGameState(GetWorld())))
			{
				GS->MulticastReliableMessageToClients(DeathMessage);
			}
		}
	}
	else
	{
		ABILITY_LOG(Warning, TEXT("UHealthComponent: Actor [%s] ran out of health, but could not die."), *GetNameSafe(GetOwner()));
	}

#endif // #if WITH_SERVER_CODE
}
