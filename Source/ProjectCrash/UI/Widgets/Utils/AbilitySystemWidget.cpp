// Copyright Samuel Reitich 2024.


#include "UI/Widgets/Utils/AbilitySystemWidget.h"

#include "AbilitySystem/CrashAbilitySystemGlobals.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "GameFramework/PlayerState.h"

void UAbilitySystemWidget::NativeConstruct()
{
	Super::NativeConstruct();

	const APlayerController* PC = GetOwningPlayer();
	const APlayerState* PS = PC ? PC->GetPlayerState<APlayerState>() : nullptr;
	OwningASC = PS ? UCrashAbilitySystemGlobals::GetCrashAbilitySystemComponentFromActor(PS) : nullptr;

	if (OwningASC)
	{
		/* Register with the ASC's delegates. */
		OwningASC->AbilityGrantedDelegate.AddDynamic(this, &UAbilitySystemWidget::OnAbilityGranted);
		OwningASC->AbilityRemovedDelegate.AddDynamic(this, &UAbilitySystemWidget::OnAbilityRemoved);
	}
}

void UAbilitySystemWidget::OnAbilityGranted(const FGameplayAbilitySpec& GrantedAbilitySpec)
{
}

void UAbilitySystemWidget::OnAbilityRemoved(const FGameplayAbilitySpec& RemovedAbilitySpec)
{
}
