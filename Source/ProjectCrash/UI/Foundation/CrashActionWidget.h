// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "CommonActionWidget.h"
#include "CrashActionWidget.generated.h"

class UCommonTextBlock;
class UEnhancedInputLocalPlayerSubsystem;
class UInputAction;

/**
 * A data table structure for defining custom display names for keys. Most keys have expected display names (e.g.
 * the "E" key is named "E"), but some may need their names overridden. E.g. we expect "LShift" instead of "Left Shift."
 */
USTRUCT()
struct FKeyOverrideRow : public FTableRowBase
{
	GENERATED_BODY()

	// Properties.

public:

	/** The key whose name should be overridden. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FKey Key;

	/** The name with which to override the key's default display name. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayKeyName = FText();



	// Utils.

public:

	/** Helper function for retrieving the row with the given key as its Key value. Returns nullptr if no such row
	 * exists. */
	static FKeyOverrideRow* GetRowFromKey(FKey InKey, UDataTable* InDataTable);
};



/**
 * An action widget that retrieves the icon of the key currently assigned to the common input action bound to this
 * widget.
 *
 * TODO: Figure out why the icon isn't appearing when the game starts, and only works after changing the input method once.
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTCRASH_API UCrashActionWidget : public UCommonActionWidget
{
	GENERATED_BODY()

	// Icon.

public:

	/** Attempts to find a key assigned to this widget's input action. If one cannot be found, falls back to the
	 * Common UI's default data table. */
	virtual FSlateBrush GetIcon() const override;

	/** The Enhanced Input Action that is associated with this Common Input action. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	const UInputAction* AssociatedInputAction;

protected:



	// Dynamic icon.

// Data.
protected:

	/** Defines custom names to use for specific keys, instead of their default engine names, when creating dynamic
	 * icons. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dynamic Icon")
	TObjectPtr<UDataTable> KeyNameOverridesTable;

	/** The brush that will be used when dynamically creating icons, instead of retrieving a brush for the key. This
	 * acts as a background for the text widget that displays the key's name. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dynamic Icon")
	FSlateBrush DynamicIconBrush;

// Widgets.

	/** The text widget used for dynamic icons to display the key's name. Must be set manually by the action widget's
	 * outer user widget. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UserInterface|HUD|Action Widget")
	TObjectPtr<UCommonTextBlock> KeyText;



	// Utils.

private:

	/** Retrieves the local enhanced input subsystem from this widget's owning local player. */
	UEnhancedInputLocalPlayerSubsystem* GetEnhancedInputSubsystem() const;
};
