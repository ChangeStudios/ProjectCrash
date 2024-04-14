// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "CrashUIMessagingSubsystem.generated.h"

class UDialogDefinition;
class UDialogWidget;

DECLARE_DELEGATE_OneParam(FUIMessagingResultSignature, EUIMessageResult /* Result */);

/**
 * Possible results of a dialog.
 */
UENUM(BlueprintType)
enum class EUIMessageResult : uint8
{
	/** The "yes" button was pressed. */
	Confirmed,

	/** The "no" button was pressed */
	Declined,

	/** The dialog was cancelled by the user. This is not always an option, as "declined" usually serves the same
	 * purpose. */
	Cancelled,

	/** The dialog was explicitly killed by the system without receiving user input. */
	Killed,

	Unknown UMETA(Hidden)
};



/**
 * Subsystem that provides various utilities for messaging within this project's UI framework, such as pushing dialogs
 * to specified layers.
 */
UCLASS(config = Game)
class PROJECTCRASH_API UCrashUIMessagingSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

	// Construction.

public:

	/** Prevents this subsystem from being created if there's an overriding implementation somewhere else. */
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;



	// Initialization.

public:

	/** Loads the dialog classes defined in the game config. */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;



	// Dialogs.

// Construction.
public:

	/** Constructs a new confirmation dialog widget with the given data and config-defined widget class. */
	void CreateConfirmationDialog(UDialogDefinition* DialogDefinition, FUIMessagingResultSignature ResultCallback = FUIMessagingResultSignature());

	/** Constructs a new error dialog widget with the given data and config-defined widget class. */
	void CreateErrorDialog(UDialogDefinition* DialogDefinition, FUIMessagingResultSignature ResultCallback = FUIMessagingResultSignature());

// Loaded classes.
private:

	/** The widget used for confirmation dialogs. */
	UPROPERTY()
	TSubclassOf<UDialogWidget> ConfirmationDialogClass;

	/** The widget used for error dialogs. */
	UPROPERTY()
	TSubclassOf<UDialogWidget> ErrorDialogClass;

// Class paths.
private:

	/** Path to the confirmation dialog widget. */
	UPROPERTY(Config)
	TSoftClassPtr<UDialogWidget> ConfirmationDialogClassPath;

	/** Path to the error dialog widget. */
	UPROPERTY(Config)
	TSoftClassPtr<UDialogWidget> ErrorDialogClassPath;
};
