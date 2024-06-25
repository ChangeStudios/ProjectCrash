// Copyright Samuel Reitich. All rights reserved.


#include "Animation/AnimNotifies/AnimNotify_TriggerGameplayEvent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"

UAnimNotify_TriggerGameplayEvent::UAnimNotify_TriggerGameplayEvent()
	: Super()
{
	// Set this notify's default color in the editor.
#if WITH_EDITORONLY_DATA
		NotifyColor = FColor(0, 128, 0, 255);
#endif
}

FString UAnimNotify_TriggerGameplayEvent::GetNotifyName_Implementation() const
{
	// Return the name of the event tag used by this notify, if it has been set.
	if (EventTag.IsValid())
	{
		return EventTag.ToString();
	}

	// Return the notify's default name if an event tag has not been set.
	return Super::GetNotifyName_Implementation();
}

void UAnimNotify_TriggerGameplayEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	// Ensure we have valid references to the executing skeletal mesh component and its owning actor.
	if (!IsValid(MeshComp) || !IsValid(MeshComp->GetOwner()))
	{
		UE_LOG(LogAnimation, Warning, TEXT("UAnimNotify_TriggerGameplayEvent: Tried to send a gameplay event in animation [%s], but the skeletal mesh component and/or its owning actor are not valid."), *GetNameSafe(Animation));
		return;
	}

	// Ensure the owner has an ASC. We can't send events to actors without an ASC.
	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(MeshComp->GetOwner());
	if (!IsValid(ASC))
	{
		UE_LOG(LogAnimation, Warning, TEXT("UAnimNotify_TriggerGameplayEvent: Tried to send a gameplay event in animation [%s], but the owning actor, [%s], does not have a valid ASC."), *GetNameSafe(Animation), *GetNameSafe(MeshComp->GetOwner()));
		return;
	}

	// Ensure the specified gameplay tag for the event is valid.
	if (!EventTag.IsValid())
	{
		UE_LOG(LogAnimation, Warning, TEXT("UAnimNotify_TriggerGameplayEvent: Tried to send a gameplay event in animation [%s] for [%s], but the gameplay event tag is not set or is not valid."), *GetNameSafe(Animation), *GetNameSafe(MeshComp->GetOwner()));
		return;
	}

	// Data needed to send the gameplay event.
	FGameplayEventData EventData = FGameplayEventData();
	EventData.Instigator = MeshComp->GetOwner();
	EventData.EventTag = EventTag;

	// Send the gameplay event to the owner of the skeletal mesh component that triggered this notify.
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshComp->GetOwner(), EventTag, EventData);

	Super::Notify(MeshComp, Animation, EventReference);
}
