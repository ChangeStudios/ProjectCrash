// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameplayMessageProcessor.generated.h"

class UObject;
namespace EEndPlayReason { enum Type : int; }

/**
 * Base class for any message processor which observes gameplay messages and potentially re-emits updates (e.g. multiple
 * assists for an elimination).
 * 
 * Note that these processors are spawned once on the server and should do their own internal filtering if they are only
 * relevant for some players.
 */
UCLASS(BlueprintType, Blueprintable, Meta = (BlueprintSpawnableComponent))
class PROJECTCRASH_API UGameplayMessageProcessor : public UActorComponent
{
	GENERATED_BODY()

	// Initialization.

public:

	/** Starts listening for relevant gameplay messages. */
	virtual void BeginPlay() override;

	/** Stops listening for relevant gameplay messages. Unregisters this component's listener handles from the message
	 * subsystem. */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:

	/**
	 * Called when this component is created. Override to specify which gameplay messages to listen to and how to
	 * respond.
	 *
	 * Use AddListenerHandle to add registered listeners to this component. These will be unregistered automatically
	 * in EndPlay.
	 */
	virtual void StartListening() {}

	/** Called when this component is destroyed. */
	virtual void StopListening() {}



	// Internals.

protected:

	/** Caches the given gameplay message listener handle to be automatically unregistered from the message subsystem
	 * when this component is destroyed. The given handle should already be registered with the message subsystem. */
	void AddListenerHandle(FGameplayMessageListenerHandle&& Handle);

private:

	/** Listener handles for relevant gameplay messages. These are automatically unregistered from the message subsystem
	 * when this component is destroyed. */
	TArray<FGameplayMessageListenerHandle> ListenerHandles;



	// Utils.

protected:

	/** Returns the server's world time in seconds. */
	double GetServerTime() const;
};
