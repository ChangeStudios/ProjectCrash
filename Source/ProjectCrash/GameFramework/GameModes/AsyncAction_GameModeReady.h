// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"
#include "AsyncAction_GameModeReady.generated.h"

class UCrashGameModeData;
class UGameModeManagerComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGameModeReadyAsyncSignature);

/**
 * Fires an event when game mode data has been loaded, and the game mode is ready.
 *
 * Will fire once on server and clients.
 */
UCLASS()
class PROJECTCRASH_API UAsyncAction_GameModeReady : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	/** Fires an event when game mode data has been loaded, and the game mode is ready. Will fire once on server and
	 * clients. */
	UFUNCTION(BlueprintCallable, Meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true"))
	static UAsyncAction_GameModeReady* WaitForGameModeReady(UObject* WorldContextObject);

	/** Initiates the listening sequence. */
	virtual void Activate() override;

public:

	/** Callback event fired when the game mode is ready (game mode data has been loaded). Takes the form of an output
	 * pin on this node. */
	UPROPERTY(BlueprintAssignable)
	FGameModeReadyAsyncSignature OnReady;

private:

	/** Called when the game state is set, if it wasn't ready when this node was called. */
	void Step1_HandleGameStateSet(AGameStateBase* GameState);
	/** Registers this node as a listener for when the game mode data is loaded. */
	void Step2_ListenForGameModeDataLoaded(AGameStateBase* GameState);
	/** Callback when the game mode data has been loaded. Immediately invokes @Step4_BroadcastReady. */
	void Step3_HandleGameModeDataLoaded(const UCrashGameModeData* CurrentGameModeData);
	/** Finally triggers this node's callback event and marks this action for destruction. */
	void Step4_BroadcastReady();

	/** Outer world this node is called in. */
	TWeakObjectPtr<UWorld> WorldPtr;
};
