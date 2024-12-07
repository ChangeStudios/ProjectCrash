// Copyright Samuel Reitich. All rights reserved.

#include "AbilitySystem/AnimNotifyState_TriggerAbilityTargeting.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "CrashGameplayTags.h"
#include "EngineLogs.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#define LOCTEXT_NAMESPACE "GameplayAbility"

UAnimNotifyState_TriggerAbilityTargeting::UAnimNotifyState_TriggerAbilityTargeting()
	: Super()
{
#if WITH_EDITORONLY_DATA
	// We'll never have an ASC in the editor.
	bShouldFireInEditor = false;

	NotifyColor = FColor(0, 128, 0, 255);
#endif // WITH_EDITORONLY_DATA
}

FString UAnimNotifyState_TriggerAbilityTargeting::GetNotifyName_Implementation() const
{
	if (AbilityIdentifier.IsValid())
	{
		FString Left, Right;
		AbilityIdentifier.ToString().Split("Identifier.", &Left, &Right);
		return "Trigger Targeting (" + Right + ")";
	}
	else
	{
		return "Trigger Targeting (Ability Not Set)";
	}
}

void UAnimNotifyState_TriggerAbilityTargeting::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	/* Never fire a targeting event without an ability identifier. This could cause lead to inadvertent effects on other
	 * abilities that use event-based targeting. */
	if (!ensureAlwaysMsgf(AbilityIdentifier.IsValid(), TEXT("UAnimNotify_TriggerAbilityTargeting: Tried to start ability targeting in animation [%s], but no ability identifier has been set."), *GetNameSafe(Animation)))
	{
		return;
	}

	if (MeshComp)
	{
		// Ensure we have a valid owning actor. This is the actor to whom the event will be sent.
		AActor* Owner = MeshComp->GetOwner();
		if (!IsValid(Owner))
		{
			UE_LOG(LogAnimation, Warning, TEXT("UAnimNotifyState_TriggerAbilityTargeting: Tried to start ability targeting in animation [%s], but the animation's owning actor are not valid."), *GetNameSafe(Animation));
			return;
		}

		// Ensure the actor to which we're sending this event has an ASC that can receive it.
		UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Owner);
		if (!IsValid(ASC))
		{
			UE_LOG(LogAnimation, Warning, TEXT("UAnimNotifyState_TriggerAbilityTargeting: Tried to start ability targeting in animation [%s], but the owning actor, [%s], does not have a valid ASC."), *GetNameSafe(Animation), *GetNameSafe(Owner));
		}

		// Construct the gameplay event payload.
		FGameplayEventData EventData = FGameplayEventData();
		EventData.Instigator = Owner;
		EventData.EventTag = CrashGameplayTags::TAG_GameplayEvent_Ability_StartTargeting;
		EventData.TargetTags.AddTag(AbilityIdentifier);

		// Send the gameplay event to the owner of the mesh component that triggered this notify.
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, EventData.EventTag, EventData);
	}
}

void UAnimNotifyState_TriggerAbilityTargeting::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	/* Never fire a targeting event without an ability identifier. This could cause lead to inadvertent effects on other
	 * abilities that use event-based targeting. */
	if (!ensureAlwaysMsgf(AbilityIdentifier.IsValid(), TEXT("UAnimNotify_TriggerAbilityTargeting: Tried to end ability targeting in animation [%s], but no ability identifier has been set."), *GetNameSafe(Animation)))
	{
		return;
	}

	if (MeshComp)
	{
		// Ensure we have a valid owning actor. This is the actor to whom the event will be sent.
		AActor* Owner = MeshComp->GetOwner();
		if (!IsValid(Owner))
		{
			UE_LOG(LogAnimation, Warning, TEXT("UAnimNotifyState_TriggerAbilityTargeting: Tried to end ability targeting in animation [%s], but the animation's owning actor are not valid."), *GetNameSafe(Animation));
			return;
		}

		// Ensure the actor to which we're sending this event has an ASC that can receive it.
		UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Owner);
		if (!IsValid(ASC))
		{
			UE_LOG(LogAnimation, Warning, TEXT("UAnimNotifyState_TriggerAbilityTargeting: Tried to end ability targeting in animation [%s], but the owning actor, [%s], does not have a valid ASC."), *GetNameSafe(Animation), *GetNameSafe(Owner));
		}

		// Construct the gameplay event payload.
		FGameplayEventData EventData = FGameplayEventData();
		EventData.Instigator = Owner;
		EventData.EventTag = CrashGameplayTags::TAG_GameplayEvent_Ability_EndTargeting;
		EventData.TargetTags.AddTag(AbilityIdentifier);

		// Send the gameplay event to the owner of the mesh component that triggered this notify.
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, EventData.EventTag, EventData);
	}
}

#undef LOCTEXT_NAMESPACE