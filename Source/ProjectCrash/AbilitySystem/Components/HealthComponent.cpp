// Copyright Samuel Reitich 2024.


#include "AbilitySystem/Components/HealthComponent.h"

#include "AbilitySystemLog.h"
#include "CrashAbilitySystemComponent.h"
#include "CrashGameplayTags.h"
#include "AbilitySystem/AttributeSets/HealthAttributeBaseValues.h"
#include "AbilitySystem/AttributeSets/HealthAttributeSet.h"
#include "GameFramework/CrashLogging.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/GameModes/Game/CrashGameMode.h"
#include "GameFramework/GameStates/CrashGameState.h"
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

void UHealthComponent::InitializeWithAbilitySystem(UCrashAbilitySystemComponent* InASC, UHealthAttributeBaseValues* InAttributeBaseValues)
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
	OutOfHealthDelegate.Broadcast(this, DamageInstigator, DamageMagnitude);

#if WITH_SERVER_CODE

	if (AbilitySystemComponent)
	{
		// Notify the game mode that the ASC's avatar has died. Death logic will work with an avatar of any actor class.
		if (ACrashGameMode* CrashGM = Cast<ACrashGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
		{
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

			CrashGM->StartDeath(DeathData);
		}

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
			if (ACrashGameState* GS = Cast<ACrashGameState>(UGameplayStatics::GetGameState(GetWorld())))
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
