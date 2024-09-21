// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "GameplayTagContainer.h"
#include "TaggedWidget.generated.h"

class UAbilitySystemComponent;
/**
 * A widget that contains a collection of gameplay tags that can be added to or removed from the owning player to
 * control the visibility of the widget.
 */
UCLASS(Abstract, Blueprintable, PrioritizeCategories = "Behavior")
class PROJECTCRASH_API UTaggedWidget : public UCommonUserWidget
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	UTaggedWidget(const FObjectInitializer& ObjectInitializer);

	/** Waits for this widget's owning controller to receive a player state, so we can start listening to its ASC. */
	virtual void NativeOnInitialized() override;

	/** Starts listening for gameplay tag changes on the owning player. */
	UFUNCTION()
	void OnPlayerStateSet(const APlayerState* NewPlayerState);

	/** Stops listening for gameplay tag changes on the owning player. */
	virtual void NativeDestruct() override;



	// Visibility.

public:

	/** Takes this widget's tags into account when trying to manually set its visibility. */
	virtual void SetVisibility(ESlateVisibility InVisibility) override;

protected:

	/** If this widget's owning player has any of these tags, this widget will be hidden (with HiddenVisibility).
	 * Otherwise, this widget's visibility will be set to VisibleVisibility. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior")
	FGameplayTagContainer HiddenByTags;

	/** Visibility to use when the owning player does not have any tags in HiddenByTags. */
	UPROPERTY(EditAnywhere, Category = "Behavior")
	ESlateVisibility ShownVisibility = ESlateVisibility::SelfHitTestInvisible;

	/** Visibility to use when the owning player has any tags in HiddenByTags. */
	UPROPERTY(EditAnywhere, Category = "Behavior")
	ESlateVisibility HiddenVisibility = ESlateVisibility::Collapsed;

private:

	/** Whether this widget wants to be visible, regardless of its tags. Driven by SetVisibility calls rather than the
	 * widget's actual visibility. */
	bool bWantsToBeVisible = true;

	/** Hides or reveals this widget depending on the tag changes. */
	UFUNCTION()
	void OnHiddenTagsChanged(const FGameplayTag ChangedTag, int32 NewCount);

	/** The ASC to which this widget is listening for tag changes. Should be owned by this widget's owning player. */
	TObjectPtr<UAbilitySystemComponent> BoundASC;

	/** Delegates bound to the gameplay tags to which we're listening for changes. Used to remove listeners upon
	 * destruction. */
	TMap<const FGameplayTag, FDelegateHandle> TagListeners;
};
