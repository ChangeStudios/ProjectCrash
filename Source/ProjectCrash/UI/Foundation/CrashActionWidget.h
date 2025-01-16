// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonActionWidget.h"
#include "GameplayTagContainer.h"
#include "CrashActionWidget.generated.h"

class UCommonTextBlock;
class UEnhancedInputLocalPlayerSubsystem;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CommonActionWidget")
	TObjectPtr<UCommonTextBlock> KeyDisplayNameWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CommonActionWidget")
	FSlateBrush KeyNameBackground;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CommonActionWidget")
	FSlateBrush DesignTimeKeyBrush;

protected:

	virtual void UpdateActionWidget() override;

private:

	FText GetKeyDisplayName();

	UEnhancedInputLocalPlayerSubsystem* GetEnhancedInputSubsystem() const;
};
