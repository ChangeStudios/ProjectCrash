// Copyright Samuel Reitich. All rights reserved.

#include "AbilitySystem/GameplayEffects/WidgetGameplayEffectComponent.h"

#include "AbilitySystemComponent.h"
#include "CrashGameplayTags.h"
#include "GameplayEffect.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/GameModes/CrashGameState.h"
#include "GameFramework/Messages/CrashGameplayEffectMessage.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/DataValidation.h"

#define LOCTEXT_NAMESPACE "GameplayEffectComponent"

void UWidgetGameplayEffectComponent::OnGameplayEffectApplied(FActiveGameplayEffectsContainer& ActiveGEContainer, FGameplayEffectSpec& GESpec, FPredictionKey& PredictionKey) const
{
	Super::OnGameplayEffectApplied(ActiveGEContainer, GESpec, PredictionKey);

	UAbilitySystemComponent* ASC = ActiveGEContainer.Owner;
	const FGameplayAbilityActorInfo* ActorInfo = ASC ? ASC->AbilityActorInfo.Get() : nullptr;
	APlayerController* PC = ActorInfo ? ActorInfo->PlayerController.Get() : nullptr;

	if (IsValid(PC) && ActorInfo->IsLocallyControlledPlayer())
	{
		UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(PC->GetWorld());

		// If the effect does not stack, or this is the first stack, create a new effect widget.
		if ((GESpec.Def && GESpec.Def->StackingType == EGameplayEffectStackingType::None) ||
			GESpec.GetStackCount() <= 1)
		{
			// Initialize the new widget with a message.
			FCrashGameplayEffectMessage GEWidgetMessage;
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
			FCrashGameplayEffectMessage GEWidgetMessage;
			GEWidgetMessage.GameplayEffectDefinition = GESpec.Def;
			GEWidgetMessage.Duration = GESpec.CalculateModifiedDuration();
			GEWidgetMessage.StackCount = GESpec.GetStackCount();
			MessageSystem.BroadcastMessage(CrashGameplayTags::TAG_Message_GameplayEffectWidget_Updated, GEWidgetMessage);

			// TODO: broadcast the message to clients (stacks aren't predicted).
			if (ACrashGameState* GS = Cast<ACrashGameState>(UGameplayStatics::GetGameState(PC->GetWorld())))
			{
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

	/* Send a message to remove the effect widget if the effect is infinite. Duration-based effects should remove
	 * themselves when their duration ends. */
	if (ActiveGE.Spec.Def && ActiveGE.Spec.Def->DurationPolicy == EGameplayEffectDurationType::Infinite)
	{
		FCrashGameplayEffectMessage GEWidgetMessage;
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
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

	const UGameplayEffect* OuterGE = GetOuterUGameplayEffect();

	// Instant effects are not supported: instant effects should not need persistent widgets to indicate their status.
	if (OuterGE->DurationPolicy == EGameplayEffectDurationType::Instant)
	{
		Context.AddError(LOCTEXT("InstantEffectsNotSupported", "Gameplay effect component \"Add Effect Widget\" does not currently support instant effects."));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}
#endif // WITH_EDITOR

#undef LOCTEXT_NAMESPACE