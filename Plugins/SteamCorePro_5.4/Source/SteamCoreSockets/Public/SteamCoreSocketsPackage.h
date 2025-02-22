/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

// Can't be #pragma once because other modules may define PACKAGE_SCOPE

// Intended to be the last include in an exported class definition
// Properly defines some members as "public to the module" vs "private to the consumer/user"

// [[ IncludeTool: Inline ]]

#undef PACKAGE_SCOPE
#ifdef STEAMCORESOCKETS_MODULE
#define PACKAGE_SCOPE public
#else
#define PACKAGE_SCOPE protected
#endif
