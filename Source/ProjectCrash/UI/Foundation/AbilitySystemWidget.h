// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "UI/Framework/CrashActivatableWidget.h"
#include "AbilitySystemWidget.generated.h"

class UCrashAbilitySystemComponent;

/** Delegate for broadcasting when this widget has bound to its owning player's ASC. */
DECLARE_MULTICAST_DELEGATE(FASCReadySignature);

/**
 * A widget that binds to its owning player's ability system component via the owning controller's player state.
 */
UCLASS(Abstract, Blueprintable)
class PROJECTCRASH_API UAbilitySystemWidget : public UCrashActivatableWidget
{
	GENERATED_BODY()

	// Initialization.

public:

	/** Binds this widget to its owner's ASC. */
	virtual void NativeConstruct() override;

	/** Called when this widget is bound to its owner's ASC. Necessary because clients will usually create their
	 * interface (including this widget) before their ASC has been initialized. */
	virtual void OnASCReady() {};



	// Utils.

protected:

	/** Attempts to retrieve and cache the new player state's owning ASC. */
	UFUNCTION()
	void OnPlayerStateChanged();

	/** This widget's owning player's ASC, cached for convenience. */
	UPROPERTY(BlueprintReadOnly)
	UCrashAbilitySystemComponent* OwningASC;
};
