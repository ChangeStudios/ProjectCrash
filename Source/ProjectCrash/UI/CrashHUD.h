// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "CrashHUD.generated.h"

/**
 * Default HUD class for this project. Responsible for hooking into the "AddWidgets" game feature action, allowing the
 * game mode to initialize the player's user interface layout and widgets.
 *
 * You should never need to derive from this class and should very rarely need to extend or modify it. The user
 * interface is driven by the CommonUI and UIExtension plugins, which initialize and manage the user interface
 * modularly using various subsystems.
 */
UCLASS()
class PROJECTCRASH_API ACrashHUD final : public AHUD
{
	GENERATED_BODY()
	
	// Construction.

public:

	/** Default constructor. */
	ACrashHUD(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());



	// Initialization.

protected:

	/** Registers this actor with the game framework component manager, so it can send game framework extension
	 * events. */
	virtual void PreInitializeComponents() override;

	/** Sends the "GameActorReady" extension event, which tells the "AddWidgets" game feature action that the HUD is
	 * ready to add the action's layouts and widgets. */
	virtual void BeginPlay() override;

	/** Unregisters this actor from the game framework component manager. This removes any layouts and widgets that
	 * were added to this HUD via game features actions. */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;



	// Debugging.

public:

	/** Adds any actors with an ability system component to the list of debug actors (as opposed to only pawns). */
	virtual void GetDebugActorList(TArray<AActor*>& InOutList) override;
};
