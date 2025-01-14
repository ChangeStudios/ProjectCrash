// Copyright Samuel Reitich. All rights reserved.


#include "UI/HUD/AbilitySystemWidgetBase.h"

#include "AbilitySystemLog.h"
#include "AbilitySystem/CrashAbilitySystemGlobals.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "GameFramework/PlayerState.h"
#include "Player/CrashPlayerController.h"

bool UAbilitySystemWidgetBase::Initialize()
{
	if (!IsDesignTime())
	{
		ACrashPlayerController* CrashPC = GetOwningPlayer<ACrashPlayerController>();

		// If the player state is ready, immediately bind to it.
		if (APlayerState* PS = CrashPC->GetPlayerState<APlayerState>())
		{
			OnPlayerStateChanged(PS);
		}
		// If the player state hasn't been set yet, wait for it before attempting to bind.
		else
		{
			CrashPC->PlayerStateChangedDelegate.AddDynamic(this, &UAbilitySystemWidgetBase::OnPlayerStateChanged);
		}
	}

	return Super::Initialize();
}

void UAbilitySystemWidgetBase::RemoveFromParent()
{
	// Stop listening for the player state to be set.
	if (ACrashPlayerController* CrashPC = GetOwningPlayer<ACrashPlayerController>())
	{
		CrashPC->PlayerStateChangedDelegate.RemoveAll(this);
	}

	Super::RemoveFromParent();
}

void UAbilitySystemWidgetBase::OnPlayerStateChanged(const APlayerState* NewPlayerState)
{
	// This widget can only be bound once.
	ACrashPlayerController* CrashPC = GetOwningPlayer<ACrashPlayerController>();
	CrashPC->PlayerStateChangedDelegate.RemoveAll(this);

	// Cache the player's ASC.
	BoundASC = UCrashAbilitySystemGlobals::GetCrashAbilitySystemComponentFromActor(NewPlayerState);

	if (!BoundASC.IsValid())
	{
		ABILITY_LOG(Warning, TEXT("Added ability system widget [%s] to player [%s], who does not have an ASC. Players without ability systems should not use ability system widgets."), *GetNameSafe(this), *GetNameSafe(GetOwningPlayer()));
		return;
	}

	OnAbilitySystemBound();
	K2_OnAbilitySystemBound();
}
