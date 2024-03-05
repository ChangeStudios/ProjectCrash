// Copyright Samuel Reitich 2024.


#include "UI/Widgets/Utils/AbilitySystemWidget.h"

#include "AbilitySystemLog.h"
#include "AbilitySystem/CrashAbilitySystemGlobals.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "GameFramework/PlayerState.h"
#include "Player/PlayerControllers/CrashPlayerController.h"

void UAbilitySystemWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// If this widget was created for a player with a valid player state, register with it immediately.
	ACrashPlayerController* CrashPC = Cast<ACrashPlayerController>(GetOwningPlayer());
	if (CrashPC->GetPlayerState<APlayerState>())
	{
		OnPlayerStateChanged();
	}
	/* If this widget's owner has not yet initialized their player state, wait until they do so the player state can be
	 * used to retrieve the player's ASC. */
	else
	{
		CrashPC->PlayerStateChangedDelegate.AddUObject(this, &UAbilitySystemWidget::OnPlayerStateChanged);
	}
}

void UAbilitySystemWidget::OnAbilityGranted(const FGameplayAbilitySpec& GrantedAbilitySpec)
{
}

void UAbilitySystemWidget::OnAbilityRemoved(const FGameplayAbilitySpec& RemovedAbilitySpec)
{
}

void UAbilitySystemWidget::OnPlayerStateChanged()
{
	const APlayerController* PC = GetOwningPlayer();
	const APlayerState* PS = PC ? PC->GetPlayerState<APlayerState>() : nullptr;

	check(PS);

	// Retrieve the new player state's ASC.
	OwningASC = UCrashAbilitySystemGlobals::GetCrashAbilitySystemComponentFromActor(PS);

	if (!OwningASC)
	{
		ABILITY_LOG(Error, TEXT("UAbilitySystemWidget: [%s] tried to bind to an ASC owned by [%s], but an ASC was not found."), *GetName(), *PS->GetName());
	}

	// Register with the new ASC's delegates.
	OwningASC->AbilityGrantedDelegate.AddDynamic(this, &UAbilitySystemWidget::OnAbilityGranted);
	OwningASC->AbilityRemovedDelegate.AddDynamic(this, &UAbilitySystemWidget::OnAbilityRemoved);

	// Broadcast that this widget has now bound (or been re-bound) to an ASC.
	ASCReadyDelegate.Broadcast();
}

