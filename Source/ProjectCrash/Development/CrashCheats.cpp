// Copyright Samuel Reitich. All rights reserved.


#include "Development/CrashCheats.h"

#include "AbilitySystemLog.h"
#include "AbilitySystem/Abilities/CrashGameplayAbility_Reset.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
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
	// Get the local player's ASC.
	const APlayerController* PC = GetPlayerController();
	const ACrashPlayerState* CrashPS = PC ? PC->GetPlayerState<ACrashPlayerState>() : nullptr;
	UCrashAbilitySystemComponent* CrashASC = CrashPS ? CrashPS->GetCrashAbilitySystemComponent() : nullptr;

	// Search for and activate any instance of the Reset gameplay ability granted to the player.
	if (IsValid(CrashASC))
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

		ABILITY_LOG(Error, TEXT("ResetPlayer was called on player [%s], but player does not the Reset gameplay ability."), *GetNameSafe(PC));
	}
}
