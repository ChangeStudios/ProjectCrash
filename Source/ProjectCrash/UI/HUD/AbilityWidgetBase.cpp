// Copyright Samuel Reitich. All rights reserved.


#include "UI/HUD/AbilityWidgetBase.h"

#include "AbilitySystem/CrashAbilitySystemGlobals.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "GameFramework/PlayerState.h"
#include "Player/CrashPlayerController.h"

bool UAbilityWidgetBase::Initialize()
{
	if (!IsDesignTime())
	{
		ACrashPlayerController* CrashPC = GetOwningPlayer<ACrashPlayerController>();
		if (ensure(CrashPC))
		{
			// If the player state is ready, immediately bind to it.
			if (APlayerState* PS = CrashPC->GetPlayerState<APlayerState>())
			{
				OnPlayerStateChanged(PS);
			}
			// If the player state hasn't been set yet, wait for it before attempting to bind.
			else
			{
				CrashPC->PlayerStateChangedDelegate.AddDynamic(this, &UAbilityWidgetBase::OnPlayerStateChanged);
			}
		}
	}

	return Super::Initialize();
}

void UAbilityWidgetBase::OnPlayerStateChanged(const APlayerState* NewPlayerState)
{
	// When the first valid player state is received, bind to its ASC.
	if (NewPlayerState && !BoundASC.Get())
	{
		BoundASC = UCrashAbilitySystemGlobals::GetCrashAbilitySystemComponentFromActor(NewPlayerState);
		ensureAlwaysMsgf(BoundASC.Get(), TEXT("Attempted to bind an ability widget [%s] to its owner [%s]'s ASC, but an ASC could not be found."), *GetPathNameSafe(this), *GetNameSafe(NewPlayerState));

		// Only bind to the first player state the owning controller gets.
		if (ACrashPlayerController* CrashPC = GetOwningPlayer<ACrashPlayerController>())
		{
			CrashPC->PlayerStateChangedDelegate.RemoveAll(this);
		}

		// Proper initialization.
		OnAbilitySystemBound();
	}
}

void UAbilityWidgetBase::OnAbilitySystemBound()
{
	ensure(BoundASC.Get());

	K2_OnAbilitySystemBound(BoundASC.Get());
}
