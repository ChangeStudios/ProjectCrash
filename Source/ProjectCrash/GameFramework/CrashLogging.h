// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "Logging/LogMacros.h"

	// Utilities.

/** Converts the given condition into a "True" or "False" FString. */
#define CONDITIONAL_STRING(bCondition) FString(bCondition ? "True" : "False")

/** Returns "Client" or "Server" depending on the given context object's authority. */
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



	// Crash log.

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



	// User interface log.

/** Generic log channel for the user interface. */
PROJECTCRASH_API DECLARE_LOG_CATEGORY_EXTERN(LogUserInterface, Log, All);