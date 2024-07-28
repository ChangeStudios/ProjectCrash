// Copyright Samuel Reitich. All rights reserved.


#include "GameFramework/Teams/CrashTeamAgentInterface.h"

#include "GameFramework/CrashLogging.h"

FTeamIdChangedSignature& ICrashTeamAgentInterface::GetTeamIdChangedDelegateChecked()
{
	FTeamIdChangedSignature* Result = GetTeamIdChangedDelegate();
	check(Result);
	return *Result;
}

void ICrashTeamAgentInterface::BroadcastIfTeamChanged(TScriptInterface<ICrashTeamAgentInterface> This, FGenericTeamId OldTeamId, FGenericTeamId NewTeamId)
{
	// Only broadcast the team change if the implementer actually changed teams.
	if (OldTeamId != NewTeamId)
	{
		const int32 OldTeamIndex = GenericTeamIdToInteger(OldTeamId);
		const int32 NewTeamIndex = GenericTeamIdToInteger(NewTeamId);

		UObject* ThisObj = This.GetObject();
		UE_LOG(LogTeams, Verbose, TEXT("[%s] %s assigned to team [%d]."), *GetClientServerContextString(ThisObj), *GetPathNameSafe(ThisObj), NewTeamIndex);

		// Broadcast the implementer's TeamChanged delegate.
		This.GetInterface()->GetTeamIdChangedDelegateChecked().Broadcast(ThisObj, OldTeamIndex, NewTeamIndex);
	}
}