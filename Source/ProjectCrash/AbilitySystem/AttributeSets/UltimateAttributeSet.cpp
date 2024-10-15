// Copyright Samuel Reitich. All rights reserved.


#include "AbilitySystem/AttributeSets/UltimateAttributeSet.h"

#include "AbilitySystemGlobals.h"
#include "AbilitySystemLog.h"
#include "CrashGameplayTags.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "GameFramework/CrashAssetManager.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Data/GlobalGameData.h"
#include "GameFramework/GameModes/GameModePropertySubsystem.h"
#include "GameFramework/Messages/CrashAbilityMessage.h"
#include "GameFramework/Teams/TeamSubsystem.h"
#include "Net/UnrealNetwork.h"

UUltimateAttributeSet::UUltimateAttributeSet() :
	UltimateCharge(0.0f)
{
	InitUltimateCharge(0.0f);
}

void UUltimateAttributeSet::PostInitProperties()
{
	Super::PostInitProperties();

	if (GetWorld() && GetOwningActor()->HasAuthority())
	{
		UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
		DamageListener = MessageSystem.RegisterListener(CrashGameplayTags::TAG_Message_Damage, this, &ThisClass::GrantUltimateChargeFromEffect);
		HealingListener = MessageSystem.RegisterListener(CrashGameplayTags::TAG_Message_Healing, this, &ThisClass::GrantUltimateChargeFromEffect);
	}
}

bool UUltimateAttributeSet::ShouldDamageGrantUltimateCharge(AActor* Instigator, AActor* Target) const
{
	// Instigator must be this attribute set's owner.
	if (Instigator != GetOwningActor())
	{
		return false;
	}

	// Must have a valid instigator and target.
	if (!Instigator || !Target)
	{
		return false;
	}

	// Players don't gain ultimate charge for damaging themselves.
	if (Instigator == Target)
	{
		return false;
	}

	// Only grant ultimate charge when damaging players or pawns, not non-pawn minions (e.g. turrets).
	if (!Target->IsA(APlayerState::StaticClass()) && !Target->IsA(APawn::StaticClass()))
	{
		return false;
	}

	// Do not grant ultimate charge while an ultimate is active.
	{
		FGameplayAbilitySpec Ability;
		float Cost;
		GetUltimateAbility(Ability, Cost);
		if (Ability.IsActive())
		{
			return false;
		}
	}

	// Players can only gain ultimate charge from damaging players on other teams.
	if (UTeamSubsystem* TeamSubsystem = GetWorld()->GetSubsystem<UTeamSubsystem>())
	{
		if (!TeamSubsystem->CanCauseDamage(Instigator, Target, false))
		{
			return false;
		}
	}
	else
	{
		ABILITY_LOG(Error, TEXT("UltimateAttributeSet, owned by [%s], tried to perform team comparison to grant ultimate charge, but there is no team subsystem."), *GetNameSafe(GetOwningActor()));
	}

	return true;
}

bool UUltimateAttributeSet::ShouldHealingGrantUltimateCharge(AActor* Instigator, AActor* Target) const
{
	// Instigator must be this attribute set's owner.
	if (Instigator != GetOwningActor())
	{
		return false;
	}

	// Must have a valid instigator and target.
	if (!Instigator || !Target)
	{
		return false;
	}

	// Only grant ultimate charge when healing players, not non-players or minions.
	if (!Target->IsA(APlayerState::StaticClass()))
	{
		return false;
	}

	// Do not grant ultimate charge while an ultimate is active.
	{
		FGameplayAbilitySpec Ability;
		float Cost;
		GetUltimateAbility(Ability, Cost);
		if (Ability.IsActive())
		{
			return false;
		}
	}

	// Players can only gain ultimate charge from healing players on other teams.
	if (UTeamSubsystem* TeamSubsystem = GetWorld()->GetSubsystem<UTeamSubsystem>())
	{
		if (!TeamSubsystem->CanCauseHealing(Instigator, Target, false))
		{
			return false;
		}
	}
	else
	{
		ABILITY_LOG(Error, TEXT("UltimateAttributeSet, owned by [%s], tried to perform team comparison to grant ultimate charge, but there is no team subsystem."), *GetNameSafe(GetOwningActor()));
	}

	return true;
}

void UUltimateAttributeSet::GrantUltimateChargeFromEffect(FGameplayTag Channel, const FCrashVerbMessage& Message)
{
	AActor* InstigatorAsActor = Cast<AActor>(Message.Instigator);
	AActor* TargetAsActor = Cast<AActor>(Message.Target);

	// Check if we should grant ultimate charge for this event.
	if (Channel.MatchesTag(CrashGameplayTags::TAG_Message_Damage) && !ShouldDamageGrantUltimateCharge(InstigatorAsActor, TargetAsActor))
	{
		return;
	}
	if (Channel.MatchesTag(CrashGameplayTags::TAG_Message_Healing) && !ShouldHealingGrantUltimateCharge(InstigatorAsActor, TargetAsActor))
	{
		return;
	}

	// Retrieve the global ultimate GE to grant ultimate charge.
	const TSubclassOf<UGameplayEffect> UltimateGE = UCrashAssetManager::GetOrLoadClass(UGlobalGameData::Get().UltimateChargeGameplayEffect_SetByCaller);

	if (!UltimateGE)
	{
		if (UGlobalGameData::Get().UltimateChargeGameplayEffect_SetByCaller.IsNull())
		{
			ABILITY_LOG(Error, TEXT("UltimateAttributeSet failed to find ultimate charge gameplay effect. UltimateChargeGameplayEffect_SetByCaller has not been set in global data."));
		}
		else
		{
			ABILITY_LOG(Error, TEXT("UltimateAttributeSet failed to load damage gameplay effect [%s]."), *UGlobalGameData::Get().UltimateChargeGameplayEffect_SetByCaller.GetAssetName());
		}

		return;
	}

	// Create an outgoing effect spec for granting ultimate charge.
	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
	EffectContext.AddInstigator(GetOwningActor(), Cast<AActor>(Message.Target));
	FGameplayEffectSpecHandle UltimateSpecHandle = ASC->MakeOutgoingSpec(UltimateGE, 1.0f, EffectContext);
	FGameplayEffectSpec* UltimateSpec = UltimateSpecHandle.Data.Get();

	if (!UltimateSpec)
	{
		ABILITY_LOG(Error, TEXT("DamageExecution failed to grant ultimate charge to instigator [%s]: unable to make outgoing spec."), *GetNameSafe(InstigatorAsActor));
		return;
	}

	// Add context tags to the effect, indicating where the ultimate charge is coming from.
	if (Channel == CrashGameplayTags::TAG_Message_Damage)
	{
		UltimateSpec->AddDynamicAssetTag(CrashGameplayTags::TAG_GameplayEffects_UltimateCharge_FromDamage);
	}
	else if (Channel == CrashGameplayTags::TAG_Message_Healing)
	{
		UltimateSpec->AddDynamicAssetTag(CrashGameplayTags::TAG_GameplayEffects_UltimateCharge_FromHealing);
	}

	// Set ultimate charge magnitude.
	UltimateSpec->SetSetByCallerMagnitude(CrashGameplayTags::TAG_GameplayEffects_SetByCaller_UltimateCharge, Message.Magnitude);

	// Apply the effect.
	ASC->ApplyGameplayEffectSpecToSelf(*UltimateSpec);
}

void UUltimateAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);

	ClampAndScaleUltimateCharge(Attribute, NewValue);
}

void UUltimateAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	ClampAndScaleUltimateCharge(Attribute, NewValue);
}

void UUltimateAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// Broadcast a message communicating the new ultimate charge on the server.
	BroadcastUltChargeChanged();
}

void UUltimateAttributeSet::ClampAndScaleUltimateCharge(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetUltimateChargeAttribute())
	{
		// Scale the applied ultimate charge by the UltimateChargeRate game mode property. 1.0 by default.
		const float UltimateChargeRate = UGameModePropertySubsystem::GetGameModeProperty(GetWorld(), CrashGameplayTags::TAG_GameMode_Property_UltimateChargeRate);
		NewValue = NewValue * UltimateChargeRate;

		// Dynamically determine the maximum ultimate charge: the cost of the player's current ultimate ability.
		FGameplayAbilitySpec UltimateAbility;
		float UltimateCostMagnitude = 0.0f;
		GetUltimateAbility(UltimateAbility, UltimateCostMagnitude);

		NewValue = FMath::Clamp(NewValue, 0.0f, UltimateCostMagnitude);
	}
}

void UUltimateAttributeSet::OnRep_UltimateCharge(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UUltimateAttributeSet, UltimateCharge, OldValue);

	// Broadcast a message communicating the new ultimate charge on clients.
	BroadcastUltChargeChanged();
}

float UUltimateAttributeSet::GetMaxUltimateCharge() const
{
	FGameplayAbilitySpec UltimateAbility;
	float UltCost = 0.0f;
	GetUltimateAbility(UltimateAbility, UltCost);
	return UltCost;
}

bool UUltimateAttributeSet::GetUltimateAbility(FGameplayAbilitySpec& OutUltimateAbility, float& CostMagnitude) const
{
	// Search for the first activatable ability that uses the UltimateCharge attribute in its cost.
	for (FGameplayAbilitySpec& Ability : GetOwningAbilitySystemComponent()->GetActivatableAbilities())
	{
		if (const UGameplayEffect* CostEffect = Ability.Ability->GetCostGameplayEffect())
		{
			for (const FGameplayModifierInfo& Modifier : CostEffect->Modifiers)
			{
				if (Modifier.Attribute == GetUltimateChargeAttribute())
				{
					OutUltimateAbility = Ability;
					float SignedCostMagnitude;
					Modifier.ModifierMagnitude.GetStaticMagnitudeIfPossible(0.0f, SignedCostMagnitude);
					CostMagnitude = FMath::Abs(SignedCostMagnitude);
					return true;
				}
			}
		}
	}

	return false;
}

void UUltimateAttributeSet::BroadcastUltChargeChanged() const
{
	if (UGameplayMessageSubsystem::HasInstance(GetWorld()))
	{
		FGameplayAbilitySpec UltimateAbility;
		float UltCost;
		bool bHasUltimate = GetUltimateAbility(UltimateAbility, UltCost);

		FCrashAbilityMessage Message;
		Message.MessageType = CrashGameplayTags::TAG_Message_Ability_CostChanged;
		Message.AbilitySpecHandle = bHasUltimate ? UltimateAbility.Handle : FGameplayAbilitySpecHandle();
		Message.ActorInfo = *GetCrashAbilitySystemComponent()->GetCrashAbilityActorInfo();
		Message.Magnitude = GetUltimateCharge();

		UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
		MessageSystem.BroadcastMessage(Message.MessageType, Message);
	}
}

void UUltimateAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UUltimateAttributeSet, UltimateCharge, COND_None, REPNOTIFY_Always);
}
