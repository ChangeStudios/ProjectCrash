// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "TaggedActivatableWidgetStack.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTCRASH_API UTaggedActivatableWidgetStack : public UCommonActivatableWidgetStack
{
	GENERATED_BODY()

public:

	/** Registers this stack with its owning player when it is created. */
	virtual void OnWidgetAddedToList(UCommonActivatableWidget& AddedWidget) override;

	/** Unregisters this stack from its owning player when it is destroyed. */
	virtual void BeginDestroy() override;

public:

	/** The tag identifying this stack. Used to push widgets to and pop widgets from this stack without requiring a
	 * reference to this widget. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stack")
	FGameplayTag StackID;
};
