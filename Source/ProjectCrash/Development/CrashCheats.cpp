// Copyright Samuel Reitich. All rights reserved.


#include "Development/CrashCheats.h"

#include "AbilitySystemLog.h"
#include "CrashGameplayTags.h"
#include "AbilitySystem/CrashAbilitySystemGlobals.h"
#include "AbilitySystem/Abilities/CrashGameplayAbility_Reset.h"
#include "AbilitySystem/AttributeSets/UltimateAttributeSet.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "AbilitySystem/Components/HealthComponent.h"
#include "GameFramework/CrashAssetManager.h"
#include "GameFramework/CrashLogging.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Data/GlobalGameData.h"
#include "Player/CrashPlayerState.h"

UCrashCheats::UCrashCheats()
{
#if UE_WITH_CHEAT_MANAGER
	// Register this cheat extension's CDO with the cheat manager.
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		UCheatManager::RegisterForOnCheatManagerCreated(FOnCheatManagerCreated::FDelegate::CreateLambda(
			[](UCheatManager* CheatManager)
			{
				CheatManager->AddCheatManagerExtension(NewObject<ThisClass>(CheatManager));
			}));
	}
#endif // UE_WITH_CHEAT_MANAGER 
}

void UCrashCheats::ResetPlayer()
{
	// Search for and activate any instance of the Reset gameplay ability granted to the player.
	if (UCrashAbilitySystemComponent* CrashASC = GetLocalASC())
	{
		TArray<FGameplayAbilitySpec> ActivatableAbilities = CrashASC->GetActivatableAbilities();

		for (FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities)
		{
			if (AbilitySpec.Ability.IsA(UCrashGameplayAbility_Reset::StaticClass()))
			{
				CrashASC->TryActivateAbility(AbilitySpec.Handle);
				return;
			}
		}

		ABILITY_LOG(Error, TEXT("ResetPlayer was called on player [%s], but player does not have the Reset gameplay ability."), *GetNameSafe(GetPlayerController()));
	}

	ABILITY_LOG(Error, TEXT("ResetPlayer was called on player [%s], but player does not have a valid ASC."), *GetNameSafe(GetPlayerController()));
}

void UCrashCheats::KillPlayer()
{
	const APlayerController* PC = GetPlayerController();
	if (const APawn* Pawn = PC->GetPawn())
	{
		// Self-destruct via health component.
		if (UHealthComponent* HealthComp = UHealthComponent::FindHealthComponent(Pawn))
		{
			HealthComp->DamageSelfDestruct();
		}
		else
		{
			UE_LOG(LogCrash, Error, TEXT("KillPlayer was called on player [%s], but player's pawn does not have a health component."), *GetNameSafe(PC));
		}
	}
	else
	{
		UE_LOG(LogCrash, Error, TEXT("KillPlayer was called on player [%s], but player does not have a pawn to kill."), *GetNameSafe(PC));
	}
}

void UCrashCheats::ChargeUltimate(float Pct)
{
	if (UCrashAbilitySystemComponent* CrashASC = GetLocalASC())
	{
		const UAttributeSet* AttributeSet = CrashASC->GetAttributeSet(UUltimateAttributeSet::StaticClass());
		const UUltimateAttributeSet* UltimateAttributeSet = AttributeSet ? Cast<UUltimateAttributeSet>(AttributeSet) : nullptr;

		if (UltimateAttributeSet)
		{
			if (const float MaxCharge = UltimateAttributeSet->GetMaxUltimateCharge())
			{
				// Create and apply an effect spec to grant the desired ultimate charge.
				const TSubclassOf<UGameplayEffect> UltimateGE = UCrashAssetManager::GetOrLoadClass(UGlobalGameData::Get().UltimateChargeGameplayEffect_SetByCaller);
				FGameplayEffectContextHandle EffectContext = CrashASC->MakeEffectContext();
				EffectContext.AddInstigator(CrashASC->GetOwnerActor(), CrashASC->GetOwnerActor());
				FGameplayEffectSpecHandle SpecHandle = CrashASC->MakeOutgoingSpec(UltimateGE, 1.0f, EffectContext);
				FGameplayEffectSpec* Spec = SpecHandle.Data.Get();
				Spec->AddDynamicAssetTag(CrashGameplayTags::TAG_GameplayEffects_UltimateCharge_FromDamage);

				const float PctToCharge = (MaxCharge * Pct);
				Spec->SetSetByCallerMagnitude(CrashGameplayTags::TAG_GameplayEffects_SetByCaller_UltimateCharge, PctToCharge);

				CrashASC->ApplyGameplayEffectSpecToSelf(*Spec);
			}
			else
			{
				ABILITY_LOG(Error, TEXT("ChargeUltimate was called on player [%s], but player does not have an ultimate ability, or the ultimate does not require a charge."), *GetNameSafe(GetPlayerController()));
			}
		}
		else
		{
			ABILITY_LOG(Error, TEXT("ChargeUltimate was called on player [%s], but player does not have an ultimate attribute set."), *GetNameSafe(GetPlayerController()));
		}
	}
	else
	{
		ABILITY_LOG(Error, TEXT("ChargeUltimate was called on player [%s], but player does not have a valid ASC."), *GetNameSafe(GetPlayerController()));
	}
}

UCrashAbilitySystemComponent* UCrashCheats::GetLocalASC() const
{
	const APlayerController* PC = GetPlayerController();
	if (IsValid(PC))
	{
		return UCrashAbilitySystemGlobals::GetCrashAbilitySystemComponentFromActor(PC->GetPlayerState<APlayerState>());
	}

	return nullptr;
}
