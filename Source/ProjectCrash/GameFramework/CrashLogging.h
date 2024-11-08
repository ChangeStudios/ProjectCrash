// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "Logging/LogMacros.h"

	// Utilities.

/** Converts the given condition into a "True" or "False" FString. */
#define CONDITIONAL_STRING(bCondition) FString(bCondition ? "True" : "False")

/** Returns "Client" or "Server" depending on the given context object's authority. The context object must be an actor
 * or an actor component. */
PROJECTCRASH_API FString GetClientServerContextString(UObject* ContextObject = nullptr);



	// Animation log.

/** Animation log channel shorthand. */
#define ANIMATION_LOG(Verbosity, Format, ...) \
{ \
	UE_LOG(LogAnimation, Verbosity, Format, ##__VA_ARGS__); \
}



	// Crash log.

/** Generic log channel used for this project. */
PROJECTCRASH_API DECLARE_LOG_CATEGORY_EXTERN(LogCrash, Log, All);

/** Log channel for modular game mode updates. */
PROJECTCRASH_API DECLARE_LOG_CATEGORY_EXTERN(LogCrashGameMode, Log, All);



	// Equipment log.

/** Log channel for the equipment system. */
PROJECTCRASH_API DECLARE_LOG_CATEGORY_EXTERN(LogEquipment, Log, All);

/** Equipment log channel shorthand. */
#define EQUIPMENT_LOG(Verbosity, Format, ...) \
{ \
	UE_LOG(LogEquipment, Verbosity, Format, ##__VA_ARGS__); \
}



	// Game phase log.

/** Log channel for the game phase subsystem. */
PROJECTCRASH_API DECLARE_LOG_CATEGORY_EXTERN(LogGamePhase, Log, All);



	// Inventory log.

/** Log channel for the inventory system. */
PROJECTCRASH_API DECLARE_LOG_CATEGORY_EXTERN(LogInventory, Log, All);

/** Inventory log channel shorthand. */
#define INVENTORY_LOG(Verbosity, Format, ...) \
{ \
	UE_LOG(LogInventory, Verbosity, Format, ##__VA_ARGS__); \
}



	// Team log.

/** Log channel for teams. */
PROJECTCRASH_API DECLARE_LOG_CATEGORY_EXTERN(LogTeams, Log, All);



	// User interface log.

/** Generic log channel for the user interface. */
PROJECTCRASH_API DECLARE_LOG_CATEGORY_EXTERN(LogUserInterface, Log, All);