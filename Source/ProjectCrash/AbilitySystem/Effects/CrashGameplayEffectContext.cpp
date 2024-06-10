// Copyright Samuel Reitich. All rights reserved.


#include "AbilitySystem/Effects/CrashGameplayEffectContext.h"

FCrashGameplayEffectContext* FCrashGameplayEffectContext::GetCrashContextFromHandle(FGameplayEffectContextHandle Handle)
{
	FGameplayEffectContext* EffectContext = Handle.Get();

	// If the effect context is valid and is a CrashGameplayEffectContext, cast it to that type and return it.
	if (EffectContext != nullptr && EffectContext->GetScriptStruct()->IsChildOf(FCrashGameplayEffectContext::StaticStruct()))
	{
		return static_cast<FCrashGameplayEffectContext*>(EffectContext);
	}

	// Return nullptr if the effect context does not exist or is of the wrong type.
	return nullptr;
}

bool FCrashGameplayEffectContext::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	FGameplayEffectContext::NetSerialize(Ar, Map, bOutSuccess);

	// Serialize any new fields here.

	return true;
}
