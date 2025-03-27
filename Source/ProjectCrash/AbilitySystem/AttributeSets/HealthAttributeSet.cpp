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
	Overhealth(0.0f),
	OverhealthBeforeAttributeChange(0.0f),
	MaxHealth(100.0f),
	MaxHealthBeforeAttributeChange(0.0f),
	DamageBoost(1.0f),
	DamageResistance(1.0f)
{
	InitHealth(100.0f);
	InitOverhealth(0.0f);
	InitMaxHealth(100.0f);
	InitDamageBoost(1.0f);
	InitDamageResistance(1.0f);
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
		const bool bIsDamageFromSelfDestruct = Data.EffectSpec.GetDynamicAssetTags().HasTagExact(CrashGameplayTags::TAG_GameplayEffects_Damage_SelfDestruct);
		if (Data.Target.HasMatchingGameplayTag(CrashGameplayTags::TAG_State_Invulnerable) && !bIsDamageFromSelfDestruct)
		{
			// Throw out any damage executions on targets that are immune to damage.
			Data.EvaluatedData.Magnitude = 0.0f;
			return false;
		}
	}

	/* Clamp damage and healing to what can actually be applied. E.g. if we're dealing 200 damage to a target with 50
	 * health, we really only want to deal 50 damage, because our other systems only care about the damage and healing
	 * we're actually we're applying (e.g. we'd only want to grant 50 ultimate charge, not 200). */
	ClampAttribute(Data.EvaluatedData.Attribute, Data.EvaluatedData.Magnitude);

	// Cache current attribute values before changes are applied.
	HealthBeforeAttributeChange = GetHealth();
	OverhealthBeforeAttributeChange = GetOverhealth();
	MaxHealthBeforeAttributeChange = GetMaxHealth();

	return true;
}

void UHealthAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	const float MinimumHealth = 0.0f;
	const float MinimumOverhealth = 0.0f;

	// Retrieve parameters used in attribute-change delegate broadcasts.
	const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetEffectContext();
	AActor* Instigator = EffectContext.GetOriginalInstigator();

	// Map Damage to -Overhealth, then -Health, and clamp. Reset the meta Damage attribute after it has been applied.
	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		/* Send a standardized verb message that other systems can observe. This is used for triggering things like
		 * hit-markers. */
		/* NOTE: If we want to add a system for shields in the future, we would add a check here to see if the owner is
		 * shielded, and then subtract from the shield's health instead, or throw out the damage entirely if the shield
		 * doesn't have health. Then, we'd still send this message to get a hitmarker, but add context to specify that a
		 * shield was hit, so we can use a different sound and hitmarker animation. */
		if (UGameplayMessageSubsystem::HasInstance(GetWorld()))
		{
			FCrashVerbMessage DamageMessage;
			DamageMessage.Verb = CrashGameplayTags::TAG_Message_Damage;
			DamageMessage.Instigator = Instigator;
			DamageMessage.InstigatorTags = *Data.EffectSpec.CapturedSourceTags.GetAggregatedTags();
			DamageMessage.Target = GetOwningActor();
			DamageMessage.TargetTags = *Data.EffectSpec.CapturedTargetTags.GetAggregatedTags();
			DamageMessage.Magnitude = Data.EvaluatedData.Magnitude;
			Data.EffectSpec.GetAllAssetTags(DamageMessage.ContextTags);

			// Broadcast the message on the server.
			UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
			MessageSystem.BroadcastMessage(DamageMessage.Verb, DamageMessage);

			// Broadcast the message to clients.
			if (ACrashGameState* GS = Cast<ACrashGameState>(UGameplayStatics::GetGameState(GetWorld())))
			{
				GS->MulticastReliableMessageToClients(DamageMessage);
			}
		}

		// Update health.
		const float DamageToOverhealth = FMath::Min(GetOverhealth(), GetDamage());
		const float RemainingDamage = FMath::Max((GetDamage() - DamageToOverhealth), 0.0f);

		if (GetOverhealth() > 0.0f)
		{
			SetOverhealth(GetOverhealth() - DamageToOverhealth);
		}

		if (RemainingDamage > 0.0f)
		{
			SetHealth(FMath::Clamp(GetHealth() - RemainingDamage, MinimumHealth, GetMaxHealth()));
		}

		SetDamage(0.0f);
	}
	// Map Healing to +Health and clamp. Reset the meta Healing attribute after it has been applied.
	else if (Data.EvaluatedData.Attribute == GetHealingAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth() + GetHealing(), MinimumHealth, GetMaxHealth()));
		SetHealing(0.0f);
	}
	// Map OverhealthDecay to -Overhealth and clamp. Reset the meta OverhealthDecay attribute after it has been applied.
	else if (Data.EvaluatedData.Attribute == GetOverhealthDecayAttribute())
	{
		SetOverhealth(FMath::Max(GetOverhealth() - GetOverhealthDecay(), MinimumOverhealth));
		SetOverhealthDecay(0.0f);
	}
	/* Clamp Health if MaxHealth is changed, in case MaxHealth falls below the current value of Health. */
	else if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), MinimumHealth, GetMaxHealth()));

		// Broadcast MaxHealth's value change on the server.
		if (GetMaxHealth() != MaxHealthBeforeAttributeChange)
		{
			MaxHealthAttributeChangedDelegate.Broadcast(Instigator, Data.EffectSpec, MaxHealthBeforeAttributeChange, GetMaxHealth());
		}
	}

	// Broadcast Health's value change.
	if (GetHealth() != HealthBeforeAttributeChange)
	{
		HealthAttributeChangedDelegate.Broadcast(Instigator, Data.EffectSpec, HealthBeforeAttributeChange, GetHealth());
	}

	// Broadcast Overhealth's value change.
	if (GetOverhealth() != OverhealthBeforeAttributeChange)
	{
		OverhealthAttributeChangedDelegate.Broadcast(Instigator, Data.EffectSpec, OverhealthBeforeAttributeChange, GetOverhealth());
	}

	// Broadcast that the target is out of health on the server. 
	if ((GetHealth() <= 0.0f) && !bOutOfHealth)
	{
		OutOfHealthAttributeDelegate.Broadcast(Instigator, Data.EffectSpec, Data.EvaluatedData.Magnitude);
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
	// Prevent Overhealth from dropping below 0.
	else if (Attribute == GetOverhealthAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	// Clamp MaxHealth to be at least 1.0.
	else if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
	}
	// Never deal more damage than can be applied (i.e. remaining health).
	else if (Attribute == GetDamageAttribute())
	{
		NewValue = FMath::Min(NewValue, GetHealth());
	}
	// Never deal more healing than can be applied (i.e. missing health).
	else if (Attribute == GetHealingAttribute())
	{
		NewValue = FMath::Min(NewValue, (GetMaxHealth() - GetHealth()));
	}
	// Never decay more Overhealth than can be removed (i.e. remaining overhealth).
	else if (Attribute == GetOverhealthDecayAttribute())
	{
		NewValue = FMath::Min(NewValue, GetOverhealth());
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
		HealthAttributeChangedDelegate.Broadcast(nullptr, FGameplayEffectSpec(), OldHealth, CurrentHealth);
	}

	// Broadcast that the target is out of health to clients.
	if ((GetHealth() <= 0.0f) && !bOutOfHealth)
	{
		OutOfHealthAttributeDelegate.Broadcast(nullptr, FGameplayEffectSpec(), EstimatedMagnitude);
	}

	// This prevents the OutOfHealthDelegate from being broadcast multiple times on each client.
	bOutOfHealth = (CurrentHealth <= 0.0f);
}

void UHealthAttributeSet::OnRep_Overhealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHealthAttributeSet, Overhealth, OldValue);

	const float CurrentOverhealth = GetOverhealth();
	const float OldOverhealth = OldValue.GetCurrentValue();

	// Broadcast the attribute value change to clients.
	if (CurrentOverhealth != OldOverhealth)
	{
		OverhealthAttributeChangedDelegate.Broadcast(nullptr, FGameplayEffectSpec(), OldOverhealth, CurrentOverhealth);
	}
}

void UHealthAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHealthAttributeSet, MaxHealth, OldValue);

	const float CurrentHealth = GetMaxHealth();
	const float OldHealth = OldValue.GetCurrentValue();

	// Broadcast the attribute value change to clients.
	if (CurrentHealth != OldHealth)
	{
		MaxHealthAttributeChangedDelegate.Broadcast(nullptr, FGameplayEffectSpec(), OldHealth, CurrentHealth);
	}
}

void UHealthAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UHealthAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHealthAttributeSet, Overhealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHealthAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHealthAttributeSet, DamageBoost, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHealthAttributeSet, DamageResistance, COND_None, REPNOTIFY_Always);
}
