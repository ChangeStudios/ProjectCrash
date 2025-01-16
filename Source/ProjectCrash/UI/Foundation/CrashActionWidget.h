// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonActionWidget.h"
#include "GameplayTagContainer.h"
#include "CrashActionWidget.generated.h"

class UCommonTextBlock;
class UEnhancedInputLocalPlayerSubsystem;
class UImage;

/**
 * EnhancedInputAction must be set. For base widgets, creating a binding so the enhanced input action can be set by
 * sub-classes (make sure not to set it during design time).
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTCRASH_API UCrashActionWidget : public UCommonActionWidget
{
	GENERATED_BODY()

public:

	virtual FSlateBrush GetIcon() const override;

	// When a predefined brush can't be found for the key, this widget's text will be set to the key's name
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CommonActionWidget")
	TObjectPtr<UCommonTextBlock> KeyDisplayNameWidget;

	// When a predefined brush can't be found for the key, this widget will be made visible to act as the background for the key's name
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CommonActionWidget")
	TObjectPtr<UImage> KeyBackgroundWidget;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CommonActionWidget", Meta = (EditCondition = "DesignTimeKey == NAME_None"))
	FSlateBrush DesignTimeBrush;
#endif

protected:

	virtual void UpdateActionWidget() override;

	virtual void UpdateKeyName();

private:

	FText GetKeyDisplayName();

	UEnhancedInputLocalPlayerSubsystem* GetEnhancedInputSubsystem() const;
};
