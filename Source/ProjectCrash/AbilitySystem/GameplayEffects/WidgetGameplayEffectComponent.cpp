// Copyright Samuel Reitich. All rights reserved.

#include "AbilitySystem/GameplayEffects/WidgetGameplayEffectComponent.h"

#include "AbilitySystemComponent.h"
#include "CrashGameplayTags.h"
#include "GameplayEffect.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"

void UWidgetGameplayEffectComponent::OnGameplayEffectApplied(FActiveGameplayEffectsContainer& ActiveGEContainer, FGameplayEffectSpec& GESpec, FPredictionKey& PredictionKey) const
{
	Super::OnGameplayEffectApplied(ActiveGEContainer, GESpec, PredictionKey);

	UAbilitySystemComponent* ASC = ActiveGEContainer.Owner;
	const FGameplayAbilityActorInfo* ActorInfo = ASC ? ASC->AbilityActorInfo.Get() : nullptr;
	APlayerController* PC = ActorInfo ? ActorInfo->PlayerController.Get() : nullptr;

	if (IsValid(PC) && ActorInfo->IsLocallyControlledPlayer())
	{
		UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(PC->GetWorld());

		if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
		{
			// If the effect does not stack, or this is the first stack, create a new effect widget.
			if ((GESpec.Def && GESpec.Def->StackingType == EGameplayEffectStackingType::None) ||
				GESpec.GetStackCount() <= 1)
			{
				// Create a widget for the new gameplay effect.
				UUIExtensionSubsystem* ExtensionSubsystem = PC->GetWorld()->GetSubsystem<UUIExtensionSubsystem>();
				FUIExtensionHandle Handle = ExtensionSubsystem->RegisterExtensionAsWidgetForContext(WidgetSlot, LocalPlayer, Widget, -1);

				// Initialize the new widget with a message.
				FGameplayEffectWidgetMessage GEWidgetMessage;
				GEWidgetMessage.ExtensionHandle = Handle;
				GEWidgetMessage.GameplayEffectDefinition = GESpec.Def;
				GEWidgetMessage.Duration = GESpec.CalculateModifiedDuration();
				GEWidgetMessage.StackCount = GESpec.GetStackCount();
				MessageSystem.BroadcastMessage(CrashGameplayTags::TAG_Message_GameplayEffectWidget_Started, GEWidgetMessage);

				// Listen for the effect's removal.
				ActiveGEContainer.OnActiveGameplayEffectRemovedDelegate.AddUObject(this, &UWidgetGameplayEffectComponent::OnGameplayEffectRemoved);
			}
			// If a new stack of an ongoing effect was applied, send a message to update the existing widget.
			else
			{
				FGameplayEffectWidgetMessage GEWidgetMessage;
				GEWidgetMessage.GameplayEffectDefinition = GESpec.Def;
				GEWidgetMessage.Duration = GESpec.CalculateModifiedDuration();
				GEWidgetMessage.StackCount = GESpec.GetStackCount();
				MessageSystem.BroadcastMessage(CrashGameplayTags::TAG_Message_GameplayEffectWidget_Updated, GEWidgetMessage);
			}
		}
	}
}

void UWidgetGameplayEffectComponent::OnGameplayEffectRemoved(const FActiveGameplayEffect& ActiveGE) const
{
	UAbilitySystemComponent* ASC = ActiveGE.Spec.GetEffectContext().GetInstigatorAbilitySystemComponent();

	if (!ensure(ASC))
	{
		return;
	}

	if (!ASC->AbilityActorInfo.IsValid() || !ASC->AbilityActorInfo->IsLocallyControlledPlayer())
	{
		return;
	}

	/* Send a message to remove the effect widget if the effect is infinite. Duration-based effects remove themselves
	 * when their duration ends. */
	if (ActiveGE.Spec.Def && ActiveGE.Spec.Def->DurationPolicy == EGameplayEffectDurationType::Infinite)
	{
		FGameplayEffectWidgetMessage GEWidgetMessage;
		GEWidgetMessage.GameplayEffectDefinition = ActiveGE.Spec.Def;
		GEWidgetMessage.Duration = ActiveGE.Spec.CalculateModifiedDuration();
		GEWidgetMessage.StackCount = ActiveGE.Spec.GetStackCount();

		UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(ASC);
		MessageSystem.BroadcastMessage(CrashGameplayTags::TAG_Message_GameplayEffectWidget_Ended, GEWidgetMessage);
	}
}

#if WITH_EDITOR
EDataValidationResult UWidgetGameplayEffectComponent::IsDataValid(class FDataValidationContext& Context) const
{
	return Super::IsDataValid(Context);
}
#endif // WITH_EDITOR