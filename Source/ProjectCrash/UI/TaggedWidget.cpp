// Copyright Samuel Reitich. All rights reserved.


#include "UI/TaggedWidget.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameFramework/PlayerState.h"

UTaggedWidget::UTaggedWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UTaggedWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!IsDesignTime())
	{
		// Start listening for any of this widget's observed tags being added or removed.
		BoundASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwningPlayerState());

		if (BoundASC)
		{
			for (const FGameplayTag& Tag : HiddenByTags)
			{
				TagListeners.Add(Tag,BoundASC->RegisterGameplayTagEvent(Tag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UTaggedWidget::OnHiddenTagsChanged));
			}
		}

		// Initialize our visibility.
		SetVisibility(GetVisibility());
	}
}

void UTaggedWidget::NativeDestruct()
{
	// Unregister from every tag we were listening to.
	if (BoundASC)
	{
		for (auto KVP& : TagListeners)
		{
			BoundASC->UnregisterGameplayTagEvent(KVP.Value, KVP.Key);
		}
	}

	Super::NativeDestruct();
}

void UTaggedWidget::SetVisibility(ESlateVisibility InVisibility)
{
	Super::SetVisibility(InVisibility);
}

void UTaggedWidget::OnHiddenTagsChanged(const FGameplayTag ChangedTag, int32 NewCount)
{
}
