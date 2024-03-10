// Copyright Samuel Reitich 2024.


#include "UI/Widgets/HUD/AbilitySlotWidget.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Abilities/CrashGameplayAbilityBase.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "Components/Image.h"
#include "GameFramework/PlayerState.h"

void UAbilitySlotWidget::BindSlotToAbility(UGameplayAbility* Ability, UCrashAbilitySystemComponent* OwningASC)
{
	if (!Ability)
	{
		return;
	}

	BoundAbility = Ability;
	BoundASC = OwningASC;

	if (UCrashGameplayAbilityBase* CrashAbility = Cast<UCrashGameplayAbilityBase>(Ability))
	{
		// Update the ability icon.
		if (AbilityIcon)
		{
			AbilityIcon->SetBrushFromTexture(CrashAbility->AbilityIcon, true);
		}

		// Bind to the ability's cooldown.
		CrashAbility->AbilityCooldownStartedDelegate.AddDynamic(this, &UAbilitySlotWidget::OnCooldownStarted);

		// Bind to when this ability is activated and ended.
		CrashAbility->AbilityActivatedDelegate.AddDynamic(this, &UAbilitySlotWidget::OnAbilityActivated);
		CrashAbility->AbilityEndedDelegate.AddDynamic(this, &UAbilitySlotWidget::OnAbilityEnded);

		/* Bind to when an ability fails to activate. We'll check in the callback if the ability that failed is the one
		 * bound to this widget. */
		if (OwningASC)
		{
			OwningASC->AbilityFailedCallbacks.AddUObject(this, &UAbilitySlotWidget::OnAbilityFailed);
		}
	}
}

void UAbilitySlotWidget::OnAbilityActivated(UGameplayAbility* Ability)
{
	const APlayerState* PS = Cast<APlayerState>(Ability->GetOwningActorFromActorInfo());

	/* Call this widget's blueprint logic for when its ability is activated, if this widget is owned by the player that
	 * activated the ability. */
	if (PS && GetOwningPlayerState()->GetPlayerId() == PS->GetPlayerId())
	{
		K2_OnAbilityActivated(Ability);
	}
}

void UAbilitySlotWidget::OnAbilityEnded(UGameplayAbility* Ability)
{
	const APlayerState* PS = Cast<APlayerState>(Ability->GetOwningActorFromActorInfo());

	/* Call this widget's blueprint logic for when its ability ends, if this widget is owned by the player that
	 * originally activated the ending ability. */
	if (PS && GetOwningPlayerState()->GetPlayerId() == PS->GetPlayerId())
	{
		K2_OnAbilityEnded(Ability);
	}
}

void UAbilitySlotWidget::OnCooldownStarted(const FActiveGameplayEffect& CooldownGameplayEffect)
{
	const APlayerState* PS = Cast<APlayerState>(CooldownGameplayEffect.Handle.GetOwningAbilitySystemComponent()->GetOwnerActor());

	/* Call this widget's blueprint logic for when the cooldown starts, if this widget is owned by the player that
	 * caused the ability's cooldown (i.e. the player that activated the ability). */
	if (PS && GetOwningPlayerState()->GetPlayerId() == PS->GetPlayerId())
	{
		K2_OnCooldownStarted(CooldownGameplayEffect.GetDuration());
	}
}

void UAbilitySlotWidget::OnAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason)
{
	// Cast to UCrashGameplayAbilityBase to retrieve the CDO.
	const UCrashGameplayAbilityBase* CrashAbility = Cast<UCrashGameplayAbilityBase>(Ability);

	// Call this widget's blueprint logic for when its ability fails to activate, if this widget's ability failed.
	if (CrashAbility && CrashAbility->GetAbilityCDO() == BoundAbility && GetOwningPlayerState() == Ability->GetOwningActorFromActorInfo())
	{
		K2_OnAbilityFailed(Ability, FailureReason);
	}
}
