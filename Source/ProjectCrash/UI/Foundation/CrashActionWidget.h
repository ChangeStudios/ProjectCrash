// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "CommonActionWidget.h"
#include "CrashActionWidget.generated.h"

class UEnhancedInputLocalPlayerSubsystem;
class UInputAction;

/**
 * An action widget that will get the icon of key that is currently assigned to the common input action on this widget
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTCRASH_API UCrashActionWidget : public UCommonActionWidget
{
	GENERATED_BODY()

public:

	//~ Begin UCommonActionWidget interface
	virtual FSlateBrush GetIcon() const override;
	//~ End of UCommonActionWidget interface

	/** The Enhanced Input Action that is associated with this Common Input action. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	const TObjectPtr<UInputAction> AssociatedInputAction;

private:

	UEnhancedInputLocalPlayerSubsystem* GetEnhancedInputSubsystem() const;
};
