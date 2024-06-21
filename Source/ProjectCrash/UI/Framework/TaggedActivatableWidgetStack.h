// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "TaggedActivatableWidgetStack.generated.h"

/**
 * An activatable widget stack that can be identified by a gameplay tag. Used to push and pop widgets to layers
 * without needing direct reference.
 */
UCLASS()
class PROJECTCRASH_API UTaggedActivatableWidgetStack : public UCommonActivatableWidgetStack
{
	GENERATED_BODY()

public:

	/** The tag identifying this widget stack. Used to match push widgets to and pop widgets from this stack without
	 * a direct reference. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stack")
	FGameplayTag StackID;
};
