// Copyright Samuel Reitich. All rights reserved.


#include "UI/TaggedWidget.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameFramework/PlayerState.h"
#include "Player/CrashPlayerController.h"

UTaggedWidget::UTaggedWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UTaggedWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

#if WITH_EDITOR
	if (IsDesignTime())
	{
		return;
	}
#endif // WITH_EDITOR

	if (ensure(GetOwningPlayer()))
	{
		if (ACrashPlayerController* CrashPC = Cast<ACrashPlayerController>(GetOwningPlayer()))
		{
			CrashPC->PlayerStateChangedDelegate.AddDynamic(this, &UTaggedWidget::OnPlayerStateSet);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Created tagged widget [%s] for player [%s], who does not have a CrashPlayerController. The widget will not be able to listen for gameplay tags to control its visibility."), *GetPathNameSafe(this), *GetNameSafe(GetOwningPlayer()));
		}
	}
}

void UTaggedWidget::OnPlayerStateSet(const APlayerState* NewPlayerState)
{
	if (NewPlayerState && !BoundASC)
	{
		// Start listening for any of this widget's observed tags being added or removed.
		// BoundASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwningPlayerState());
		BoundASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(NewPlayerState);

		if (BoundASC)
		{
			for (const FGameplayTag& Tag : HiddenByTags)
			{
				TagListeners.Add(Tag,BoundASC->RegisterGameplayTagEvent(Tag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UTaggedWidget::OnHiddenTagsChanged));
			}
		}
	}

	// Initialize our visibility.
	SetVisibility(GetVisibility());
}

void UTaggedWidget::NativeDestruct()
{
	// Unregister from every tag we were listening to.
	if (BoundASC)
	{
		for (auto KVP : TagListeners)
		{
			BoundASC->UnregisterGameplayTagEvent(KVP.Value, KVP.Key);
		}
	}

	Super::NativeDestruct();
}

void UTaggedWidget::SetVisibility(ESlateVisibility InVisibility)
{
#if WITH_EDITOR
	if (IsDesignTime())
	{
		Super::SetVisibility(InVisibility);
		return;
	}
#endif

	/* Our actual visibility takes into account our tags, so we need to separately track whether this widget should be
	 * visible in the first place. */
	bWantsToBeVisible = ConvertSerializedVisibilityToRuntime(InVisibility).IsVisible();

	// Cache this widget's desired visibility for when we update it using our tags.
	if (bWantsToBeVisible)
	{
		ShownVisibility = InVisibility;
	}
	else
	{
		HiddenVisibility = InVisibility;
	}

	const bool bHasHiddenTags = (BoundASC ? BoundASC->HasAnyMatchingGameplayTags(HiddenByTags) : false);

	// Apply the desired visibility, also taking into account the current tags.
	const ESlateVisibility DesiredVisibility = (bWantsToBeVisible && !bHasHiddenTags) ? ShownVisibility : HiddenVisibility;
	if (GetVisibility() != DesiredVisibility)
	{
		Super::SetVisibility(DesiredVisibility);
	}
}

void UTaggedWidget::OnHiddenTagsChanged(const FGameplayTag ChangedTag, int32 NewCount)
{
	const bool bHasHiddenTags = (BoundASC ? BoundASC->HasAnyMatchingGameplayTags(HiddenByTags) : false);

	/* This widget is only visible when it wants to be visible (from initialization and any SetVisibility calls) AND
	 * isn't hidden by tags. */
	const ESlateVisibility DesiredVisibility = (bWantsToBeVisible && !bHasHiddenTags) ? ShownVisibility : HiddenVisibility;
	if (GetVisibility() != DesiredVisibility)
	{
		Super::SetVisibility(DesiredVisibility);
	}
}
