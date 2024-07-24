// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "UObject/Interface.h"

#include "CrashTeamAgentInterface.generated.h"

template <typename InterfaceType> class TScriptInterface;

/** Delegate for broadcasting when an object changes teams. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTeamIdChangedSignature, UObject*, ObjectChangingTeam, int32, OldTeamId, int32, NewTeamId);

/** Converts a team ID into an integer. Returns INDEX_NONE for NoTeam. */
inline int32 GenericTeamIdToInteger(FGenericTeamId Id)
{
	return (Id == FGenericTeamId::NoTeam) ? INDEX_NONE : (int32)Id;
}

/** Converts an integer to a team ID. Returns NoTeam for INDEX_NONE. */
inline FGenericTeamId IntegerToGenericTeamId(int32 Id)
{
	return (Id == INDEX_NONE) ? FGenericTeamId::NoTeam : FGenericTeamId((uint8)Id);
}



/**
 * An interface for actors that can be associated with teams. Can be used to assign actors to a team, like a player
 * state or AI character, and to allow actors to receive team events, like a player's avatar actor.
 */
UINTERFACE(Meta = (CannotImplementInterfaceInBlueprint))
class UCrashTeamAgentInterface : public UGenericTeamAgentInterface
{
	GENERATED_BODY()
};



class PROJECTCRASH_API ICrashTeamAgentInterface : public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:

	/** Returns the delegate fired when this object changes teams. */
	virtual FOnTeamIdChangedSignature* GetOnTeamIdChangedDelegate() { return nullptr; }

	/** Returns the delegate fired when this object changes teams. Performs a check that the delegate is valid. */
	FOnTeamIdChangedSignature& GetTeamIdChangedDelegateChecked();

	/** Broadcasts that this interface's implementer changed teams. Only fires if the given team IDs are different. */
	static void BroadcastIfTeamChanged(TScriptInterface<ICrashTeamAgentInterface> This, FGenericTeamId OldTeamId, FGenericTeamId NewTeamId);
};
