// Copyright Samuel Reitich. All rights reserved.

#include "Equipment/AnimNotify_TriggerEquipmentEvent.h"

#include "EquipmentEventInterface.h"
#include "GameFramework/CrashLogging.h"

UAnimNotify_TriggerEquipmentEvent::UAnimNotify_TriggerEquipmentEvent()
{
#if WITH_EDITORONLY_DATA
	// Equipment won't be visible in the animation editor.
	bShouldFireInEditor = false;

	NotifyColor = FColor(75, 225, 75);
#endif
}

FString UAnimNotify_TriggerEquipmentEvent::GetNotifyName_Implementation() const
{
	if (EventTag.IsValid())
	{
		return ("Trigger " + EventTag.ToString());
	}

	return "(Unset) Trigger Equipment Event";
}

void UAnimNotify_TriggerEquipmentEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!EventTag.IsValid() || !IsValid(MeshComp))
	{
		return;
	}

	if (AActor* Owner = MeshComp->GetOwner())
	{
		// Route the event to the animation's owning actor if it implements IEquipmentEventInterface.
		if (Owner->Implements<UEquipmentEventInterface>())
		{
			IEquipmentEventInterface::Execute_ProcessEquipmentEvent(Owner, EventTag);
		}
		else
		{
			ANIMATION_LOG(Error, TEXT("Animation [%s] tried to send equipment event to owner [%s], but it does not implement EquipmentEventInterface."), *GetNameSafe(Animation), *GetNameSafe(Owner));
		}
	}
}
