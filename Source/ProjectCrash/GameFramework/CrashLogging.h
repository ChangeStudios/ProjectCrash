// Copyright Samuel Reitich 2024.

#pragma once

#include "Logging/LogMacros.h"

	// Utilities.

/** Converts the given condition into a "True" or "False" FString. */
#define CONDITIONAL_STRING(bCondition) FString(bCondition ? "True" : "False")

/** Returns an FString indicating whether the given object is server-authoritative. */
#define AUTHORITY_STRING(ContextObject) FString(ContextObject->HasAuthority() ? "SERVER" : "CLIENT")



	// Animation log.

/** Animation log channel shorthand. */
#define ANIMATION_LOG(Verbosity, Format, ...) \
{ \
	UE_LOG(LogAnimation, Verbosity, Format, ##__VA_ARGS__); \
}



	// Equipment log.

/** Log channel for the equipment system. */
PROJECTCRASH_API DECLARE_LOG_CATEGORY_EXTERN(LogEquipment, Log, All);

/** Equipment log channel shorthand. */
#define EQUIPMENT_LOG(Verbosity, Format, ...) \
{ \
	UE_LOG(LogEquipment, Verbosity, Format, ##__VA_ARGS__); \
}