// Copyright Samuel Reitich 2024.

#pragma once

#include "GameplayEffectTypes.h"

#include "CrashGameplayEffectContext.generated.h"

/**
 * The struct for all gameplay effect contexts for this project. This provides data about gameplay effects for
 * executions to use. It can be retrieved from a context handle with GetCrashContextFromHandle.
 */
USTRUCT()
struct FCrashGameplayEffectContext: public FGameplayEffectContext
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	FCrashGameplayEffectContext() {}

	/** Constructor providing an instigator (e.g. a player that's throwing a grenade) and an effect causer (e.g. the
	 * grenade that's dealing damage). */
	FCrashGameplayEffectContext(AActor* InInstigator, AActor* InEffectCauser) : FGameplayEffectContext(InInstigator, InEffectCauser) {}

	/** Creates the given handle's context cast to CrashGameplayEffectContext. */
	static PROJECTCRASH_API FCrashGameplayEffectContext* GetCrashContextFromHandle(FGameplayEffectContextHandle Handle);

	/** Returns this structure's static structure. */
	virtual UScriptStruct* GetScriptStruct() const override { return FCrashGameplayEffectContext::StaticStruct(); }

	/** Performs a deep copy of this struct. */
	virtual FGameplayEffectContext* Duplicate() const override
	{
		FCrashGameplayEffectContext* NewContext = new FCrashGameplayEffectContext();
		*NewContext = *this;
		if (GetHitResult())
		{
			// Does a deep copy of the hit result.
			NewContext->AddHitResult(*GetHitResult(), true);
		}
		return NewContext;
	}

	/** If new fields are added to this structure, serializes them here. */
	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess) override;
};


/** Enable net serialization. */
template<>
struct TStructOpsTypeTraits<FCrashGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FCrashGameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true
	};
};