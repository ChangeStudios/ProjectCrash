// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "UI/MessagingSubsystem/CrashUIMessagingSubsystem.h"
#include "AsyncAction_CreateConfirmation.generated.h"

class UDialogDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUIMessagingResultMulticastSignature, EUIMessageResult, Result);

/**
 * Triggers an asynchronous confirmation dialog in blueprints that provides callbacks for the user's response.
 */
UCLASS()
class PROJECTCRASH_API UAsyncAction_CreateConfirmation : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "UserInterface|Actions", Meta = (BlueprintInternalUseOnly = "true", WorldContext = "InWorldContextObject", Keywords = "Dialog"))
	static UAsyncAction_CreateConfirmation* CreateConfirmation(UObject* InWorldContextObject, UDialogDefinition* DialogDefinition);

	virtual void Activate() override;

public:

	UPROPERTY(BlueprintAssignable, DisplayName = "OnResult")
	FUIMessagingResultMulticastSignature ResultDelegate;

private:

	void HandleConfirmationResult(EUIMessageResult ConfirmationResult);

	UPROPERTY(Transient)
	TObjectPtr<UObject> WorldContextObject;

	UPROPERTY(Transient)
	TObjectPtr<ULocalPlayer> TargetLocalPlayer;

	UPROPERTY(Transient)
	TObjectPtr<UDialogDefinition> DialogDefinition;
};
