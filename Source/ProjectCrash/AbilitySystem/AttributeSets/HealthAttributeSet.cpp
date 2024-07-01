// Copyright Samuel Reitich. All rights reserved.


#include "AbilitySystem/AttributeSets/HealthAttributeSet.h"

#include "AbilitySystemLog.h"
#include "GameplayEffectExtension.h"
#include "CrashGameplayTags.h"
#include "GameFramework/CrashLogging.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/GameModes/CrashGameState.h"
#include "GameFramework/Messages/CrashVerbMessage.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(HealthAttributeSet)

UHealthAttributeSet::UHealthAttributeSet() :
	Health(100.0f),
	bOutOfHealth(false),
	HealthBeforeAttributeChange(0.0f),
	MaxHealth(100.0f),
	MaxHealthBeforeAttributeChange(0.0f)
{
	InitHealth(100.0f);
	InitMaxHealth(100.0f);
}

bool UHealthAttributeSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
	if (!Super::PreGameplayEffectExecute(Data))
	{
		return false;
	}

	// Check damage immunity if damage is being applied.
	if (Data.EvaluatedData.Attribute == GetDamageAttribute() && Data.EvaluatedData.Magnitude > 0.0f)
	{
		// The self-destruction state overrides any damage invulnerabilities.
		const bool bIsDamageFromSelfDestruct = Data.EffectSpec.GetDynamicAssetTags().HasTagExact(CrashGameplayTags::TAG_Effects_Damage_SelfDestruct);
		if (Data.Target.HasMatchingGameplayTag(CrashGameplayTags::TAG_State_ImmuneToDamage) && !bIsDamageFromSelfDestruct)
		{
			// Throw out any damage executions on targets that are immune to damage.
			Data.EvaluatedData.Magnitude = 0.0f;
			return false;
		}
	}

	// Cache current attribute values before changes are applied.
	HealthBeforeAttributeChange = GetHealth();
	MaxHealthBeforeAttributeChange = GetMaxHealth();

	return true;
}

void UHealthAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	const float MinimumHealth = 0.0f;

	// Retrieve parameters used in attribute-change delegate broadcasts.
	const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetEffectContext();
	AActor* Instigator = EffectContext.GetOriginalInstigator();
	AActor* Causer = EffectContext.GetEffectCauser();

	// Map Damage to -Health and clamp. Reset the meta Damage attribute after it has been applied.
	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		/* Send a standardized verb message that other systems can observe. This is used for triggering things like
		 * hit-markers. */
		if (UGameplayMessageSubsystem::HasInstance(GetWorld()))
		{
			FCrashVerbMessage DamageMessage;
			DamageMessage.Verb = CrashGameplayTags::TAG_Message_Damage;
			DamageMessage.Instigator = Instigator;
			DamageMessage.InstigatorTags = *Data.EffectSpec.CapturedSourceTags.GetAggregatedTags();
			DamageMessage.Target = GetOwningActor();
			DamageMessage.TargetTags = *Data.EffectSpec.CapturedTargetTags.GetAggregatedTags();
			DamageMessage.Magnitude = Data.EvaluatedData.Magnitude;

			// Broadcast the message on the server.
			UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
			MessageSystem.BroadcastMessage(DamageMessage.Verb, DamageMessage);

			// TODO: Broadcast the message to clients.
			if (ACrashGameState* GS = Cast<ACrashGameState>(UGameplayStatics::GetGameState(GetWorld())))
			{
				// GS->MulticastReliableMessageToClients(DamageMessage);
			}
		}

		// Update health.
		SetHealth(FMath::Clamp(GetHealth() - GetDamage(), MinimumHealth, GetMaxHealth()));
		SetDamage(0.0f);
	}
	// Map Healing to +Health and clamp. Reset the meta Healing attribute after it has been applied.
	else if (Data.EvaluatedData.Attribute == GetHealingAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth() + GetHealing(), MinimumHealth, GetMaxHealth()));
		SetHealing(0.0f);
	}
	/* Clamp Health if MaxHealth is changed, in case MaxHealth falls below the current value of Health. MaxHealth can
	 * be modified with executions, but it is easier and more preferable to modify it directly. */
	else if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), MinimumHealth, GetMaxHealth()));

		// Broadcast MaxHealth's value change on the server.
		if (GetMaxHealth() != MaxHealthBeforeAttributeChange)
		{
			MaxHealthAttributeChangedDelegate.Broadcast(Instigator, Causer, Data.EffectSpec, MaxHealthBeforeAttributeChange, GetMaxHealth());
		}

		ABILITY_LOG(Warning, TEXT("Attribute MaxHealth was modified from an execution. This attribute can be modified directly, without executions. Modification was still processed."));
	}
	// No other attributes should be modified with executions.
	else
	{
		ABILITY_LOG(Warning, TEXT("Attempted to modify attribute [%s] with an execution. This attribute cannot be directly modified with executions."), *Data.EvaluatedData.Attribute.GetName())
	}

	// Broadcast Health's value change.
	if (GetHealth() != HealthBeforeAttributeChange)
	{
		HealthAttributeChangedDelegate.Broadcast(Instigator, Causer, Data.EffectSpec, HealthBeforeAttributeChange, GetHealth());
	}

	// Broadcast that the target is out of health on the server. 
	if ((GetHealth() <= 0.0f) && !bOutOfHealth)
	{
		OutOfHealthAttributeDelegate.Broadcast(Instigator, Causer, Data.EffectSpec, Data.EvaluatedData.Magnitude);
	}

	// This prevents the OutOfHealthDelegate from being broadcast multiple times.
	bOutOfHealth = (GetHealth() <= 0.0f);
}

void UHealthAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);

	ClampAttribute(Attribute, NewValue);
}

void UHealthAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	ClampAttribute(Attribute, NewValue);
}

void UHealthAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	// Clamp Health to MaxHealth if it is greater than MaxHealth's new value.
	if (Attribute == GetMaxHealthAttribute())
	{
		if (GetHealth() > NewValue)
		{
			UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponentChecked();
			ASC->ApplyModToAttribute(GetHealthAttribute(), EGameplayModOp::Override, NewValue);
		}
	}

	// Reset bOutOfHealth is the target's health has been changed (i.e. they have respawned).
	if (bOutOfHealth && (GetHealth() > 0.0f))
	{
		bOutOfHealth = false;
	}
}

void UHealthAttributeSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	// Clamp Health between 0 and MaxHealth.
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	// Clamp MaxHealth to be at least 1.0.
	else if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
	}
}

void UHealthAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHealthAttributeSet, Health, OldValue);

	const float CurrentHealth = GetHealth();
	const float OldHealth = OldValue.GetCurrentValue();
	const float EstimatedMagnitude = CurrentHealth - OldHealth;

	// Broadcast the attribute value change to clients.
	if (CurrentHealth != OldHealth)
	{
		HealthAttributeChangedDelegate.Broadcast(nullptr, nullptr, FGameplayEffectSpec(), OldHealth, CurrentHealth);
	}

	// Broadcast that the target is out of health to clients.
	if ((GetHealth() <= 0.0f) && !bOutOfHealth)
	{
		OutOfHealthAttributeDelegate.Broadcast(nullptr, nullptr, FGameplayEffectSpec(), EstimatedMagnitude);
	}

	// This prevents the OutOfHealthDelegate from being broadcast multiple times on each client.
	bOutOfHealth = (CurrentHealth <= 0.0f);
}

void UHealthAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHealthAttributeSet, MaxHealth, OldValue);

	const float CurrentHealth = GetHealth();
	const float OldHealth = OldValue.GetCurrentValue();

	// Broadcast the attribute value change to clients.
	if (CurrentHealth != OldHealth)
	{
		MaxHealthAttributeChangedDelegate.Broadcast(nullptr, nullptr, FGameplayEffectSpec(), OldHealth, CurrentHealth);
	}
}

void UHealthAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UHealthAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHealthAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
}