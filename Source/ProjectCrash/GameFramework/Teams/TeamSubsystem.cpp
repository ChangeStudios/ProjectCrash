// Copyright Samuel Reitich. All rights reserved.


#include "GameFramework/Teams/TeamSubsystem.h"

#include "CrashTeamAgentInterface.h"
#include "GenericTeamAgentInterface.h"
#include "TeamCheats.h"
#include "TeamInfo.h"
#include "GameFramework/CheatManager.h"
#include "Player/CrashPlayerState.h"

void UTeamSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Adds Team Cheats to the given cheat manager.
	auto AddTeamCheats = [](UCheatManager* CheatManager)
	{
		CheatManager->AddCheatManagerExtension(NewObject<UTeamCheats>(CheatManager));
	};

	// Add Team Cheats to the cheat manager when it's created.
	CheatManagerRegistrationHandle = UCheatManager::RegisterForOnCheatManagerCreated(FOnCheatManagerCreated::FDelegate::CreateLambda(AddTeamCheats));
}

void UTeamSubsystem::Deinitialize()
{
	// Stop listening for the cheat manager to be created.
	UCheatManager::UnregisterFromOnCheatManagerCreated(CheatManagerRegistrationHandle);

	Super::Deinitialize();
}

bool UTeamSubsystem::RegisterTeam(ATeamInfo* TeamInfo)
{
	if (ensure(TeamInfo))
	{
		// Make sure the team has a valid ID.
		const int32 TeamId = TeamInfo->GetTeamId();
		if (ensure(TeamId != INDEX_NONE))
		{
			// Register the team by adding it to the TeamMap.
			TeamMap.FindOrAdd(TeamId);
			TeamMap[TeamId] = TeamInfo;

			return true;
		}
	}

	return false;
}

bool UTeamSubsystem::UnregisterTeam(ATeamInfo* TeamInfo)
{
	if (ensure(TeamInfo))
	{
		// Make sure the team has a valid ID.
		const int32 TeamId = TeamInfo->GetTeamId();
		if (ensure(TeamId != INDEX_NONE))
		{
			/* Unregister the team by clearing its info actor. We don't remove the entry so new teams don't try to re-use
			 * the unregistered team's ID. */
			if (TeamMap.Contains(TeamId))
			{
				TeamMap[TeamId] = nullptr;

				return true;
			}
		}
	}

	return false;
}

bool UTeamSubsystem::ChangeTeamForActor(AActor* ActorToChange, int32 NewTeamIndex)
{
	const FGenericTeamId NewTeamId = IntegerToGenericTeamId(NewTeamIndex);

	/* If an associated player state can be found for the given actor, change the actor's team through their player
	 * state. */
	if (ACrashPlayerState* CrashPS = const_cast<ACrashPlayerState*>(FindPlayerStateFromActor(ActorToChange)))
	{
		CrashPS->SetGenericTeamId(NewTeamId);
		return true;
	}

	/* If an associated player state can't be found for the actor, try to change the actor's team directly via the
	 * agent interface. */
	if (ICrashTeamAgentInterface* ActorAsTeamAgent = Cast<ICrashTeamAgentInterface>(ActorToChange))
	{
		ActorAsTeamAgent->SetGenericTeamId(NewTeamId);
	}

	return false;
}

int32 UTeamSubsystem::FindTeamFromObject(const UObject* Object) const
{
	// Try to retrieve the team from the object directly, if it's a team agent.
	if (const ICrashTeamAgentInterface* ObjectAsTeamAgent = Cast<ICrashTeamAgentInterface>(Object))
	{
		return GenericTeamIdToInteger(ObjectAsTeamAgent->GetGenericTeamId());
	}

	if (const AActor* ObjectAsActor = Cast<const AActor>(Object))
	{
		// Try to retrieve the team from the object's instigator.
		if (const ICrashTeamAgentInterface* InstigatorAsTeamAgent = Cast<ICrashTeamAgentInterface>(ObjectAsActor->GetInstigator()))
		{
			return GenericTeamIdToInteger(InstigatorAsTeamAgent->GetGenericTeamId());
		}

		// Special case for TeamInfo actors, since they don't actually implement the team agent interface.
		if (const ATeamInfo* TeamInfo = Cast<ATeamInfo>(ObjectAsActor))
		{
			return TeamInfo->GetTeamId();
		}

		// Try to retrieve the team from the object's player state.
		if (const ACrashPlayerState* CrashPS = FindPlayerStateFromActor(ObjectAsActor))
		{
			return CrashPS->GetTeamId();
		}
	}

	return INDEX_NONE;
}

void UTeamSubsystem::FindTeamFromObject(const UObject* Object, bool& bIsOnTeam, int32& TeamId) const
{
	// BP wrapper for FindTeamFromObject.
	TeamId = FindTeamFromObject(Object);
	bIsOnTeam = (TeamId != INDEX_NONE);
}

const ACrashPlayerState* UTeamSubsystem::FindPlayerStateFromActor(const AActor* Actor) const
{
	if (Actor != nullptr)
	{
		// Actor is a pawn.
		if (const APawn* Pawn = Cast<const APawn>(Actor))
		{
			if (ACrashPlayerState* CrashPS = Pawn->GetPlayerState<ACrashPlayerState>())
			{
				return CrashPS;
			}
		}
		// Actor is a controller.
		else if (const AController* C = Cast<const AController>(Actor))
		{
			if (ACrashPlayerState* CrashPS = C->GetPlayerState<ACrashPlayerState>())
			{
				return CrashPS;
			}
		}
		// Actor is a player state.
		else if (const ACrashPlayerState* CrashPS = Cast<const ACrashPlayerState>(Actor))
		{
			return CrashPS;
		}
	}

	return nullptr;
}

bool UTeamSubsystem::DoesTeamExist(int32 TeamId) const
{
	return TeamMap.Contains(TeamId);
}

TArray<int32> UTeamSubsystem::GetTeamIds() const
{
	TArray<int32> Result;
	TeamMap.GenerateKeyArray(Result);
	Result.Sort();
	return Result;
}
