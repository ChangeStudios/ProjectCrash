// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "CrashUIMessagingSubsystem.generated.h"

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
UCLASS(Config = Game)
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

	void CreateConfirmationDialog();
	void CreateErrorDialog();

// Classes.
private:

	/** The widget used for confirmation dialogs. */
	UPROPERTY(Config)
	TSoftClassPtr<UDialogWidget> ConfirmationDialogClass;

	/** The widget used for error dialogs. */
	UPROPERTY(Config)
	TSoftClassPtr<UDialogWidget> ErrorDialogClass;
};
