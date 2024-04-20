// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "CommonActionWidget.h"
#include "CrashActionWidget.generated.h"

class UEnhancedInputLocalPlayerSubsystem;
class UInputAction;

/**
 * An action widget that retrieves the icon of the key currently assigned to the common input action bound to this
 * widget.
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTCRASH_API UCrashActionWidget : public UCommonActionWidget
{
	GENERATED_BODY()

public:

	/** Attempts to find a key assigned to this widget's input action. If one cannot be found, falls back to the
	 * Common UI's default data table. */
	virtual FSlateBrush GetIcon() const override;

	/** The Enhanced Input Action that is associated with this Common Input action. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	const UInputAction* AssociatedInputAction;

private:

	UEnhancedInputLocalPlayerSubsystem* GetEnhancedInputSubsystem() const;
};
