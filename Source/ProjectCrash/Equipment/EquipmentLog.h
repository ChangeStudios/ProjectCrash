// Copyright Samuel Reitich 2024.

#pragma once

#include "Logging/LogMacros.h"

/** Log channel for the equipment system. */
PROJECTCRASH_API DECLARE_LOG_CATEGORY_EXTERN(LogEquipment, Log, All);

// Equipment logging macro.
#define EQUIPMENT_LOG(Verbosity, Format, ...) \
{ \
UE_LOG(LogEquipment, Verbosity, Format, ##__VA_ARGS__); \
}