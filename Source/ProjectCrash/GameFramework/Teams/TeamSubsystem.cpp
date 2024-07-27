// Copyright Samuel Reitich. All rights reserved.


#include "GameFramework/Teams/TeamSubsystem.h"

#include "AbilitySystemGlobals.h"
#include "CrashTeamAgentInterface.h"
#include "GenericTeamAgentInterface.h"
#include "TeamCheats.h"
#include "TeamInfo.h"
#include "GameFramework/CheatManager.h"
#include "GameFramework/CrashLogging.h"
#include "Player/CrashPlayerState.h"

/*
 * FTeamTrackingInfo
 */
void FTeamTrackingInfo::SetTeamInfo(ATeamInfo* NewTeamInfo)
{
	check(NewTeamInfo);
	ensure((TeamInfo == nullptr) || (TeamInfo == NewTeamInfo));

	TeamInfo = NewTeamInfo;

	UTeamDisplayAsset* OldFriendlyDisplayAsset = FriendlyDisplayAsset;
	UTeamDisplayAsset* OldTeamDisplayAsset = TeamDisplayAsset;

	FriendlyDisplayAsset = NewTeamInfo->GetFriendlyDisplayAsset();
	TeamDisplayAsset = NewTeamInfo->GetTeamDisplayAsset();

	if (OldTeamDisplayAsset != TeamDisplayAsset)
	{
		TeamDisplayAssetChangedDelegate.Broadcast(TeamDisplayAsset);
	}

	if (OldFriendlyDisplayAsset != FriendlyDisplayAsset)
	{
		TeamDisplayAssetChangedDelegate.Broadcast(FriendlyDisplayAsset);
	}
}



/*
 * UTeamSubsystem
 */
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

void UTeamSubsystem::AddTeamTags(int32 TeamId, FGameplayTag Tag, int32 Count)
{
	// Error message shorthand.
	auto FailureHandler = [&](const FString& ErrorMessage)
	{
		UE_LOG(LogTeams, Error, TEXT("AddTeamTags(TeamId: %d, Tag: %s, Count: %d) failed: %s"), TeamId, *Tag.ToString(), Count, *ErrorMessage);
	};

	if (TeamMap.Contains(TeamId))
	{
		if (ATeamInfo* TeamInfo = *TeamMap.Find(TeamId))
		{
			if (TeamInfo->HasAuthority())
			{
				// Add the given number of tags to the specified team.
				TeamInfo->TeamTags.AddTags(Tag, Count);
			}
			else
			{
				FailureHandler(TEXT("attempted to add tags on a client. Team tags can only be added with authority."));
			}
		}
		else
		{
			FailureHandler(TEXT("team info has not spawned yet. Game mode data has likely not yet loaded."));
		}
	}
	else
	{
		FailureHandler(TEXT("unknown team."));
	}
}

void UTeamSubsystem::RemoveTeamTags(int32 TeamId, FGameplayTag Tag, int32 Count)
{
	// Error message shorthand.
	auto FailureHandler = [&](const FString& ErrorMessage)
	{
		UE_LOG(LogTeams, Error, TEXT("RemoveTeamTags(TeamId: %d, Tag: %s, Count: %d) failed: %s"), TeamId, *Tag.ToString(), Count, *ErrorMessage);
	};

	if (TeamMap.Contains(TeamId))
	{
		if (ATeamInfo* TeamInfo = *TeamMap.Find(TeamId))
		{
			if (TeamInfo->HasAuthority())
			{
				// Remove the given number of tags from the specified team.
				TeamInfo->TeamTags.RemoveTags(Tag, Count);
			}
			else
			{
				FailureHandler(TEXT("attempted to remove tags on a client. Team tags can only be removed with authority."));
			}
		}
		else
		{
			FailureHandler(TEXT("team info has not spawned yet. Game mode data has likely not yet loaded."));
		}
	}
	else
	{
		FailureHandler(TEXT("unknown team."));
	}
}

int32 UTeamSubsystem::GetTeamTagCount(int32 TeamId, FGameplayTag Tag) const
{
	// Return the given team's requested tag count if it exists.
	if (ATeamInfo* TeamInfo = *TeamMap.Find(TeamId))
	{
		return TeamInfo->TeamTags.GetTagCount(Tag);
	}
	// The team cannot be found.
	else if (!TeamMap.Contains(TeamId))
	{
		UE_LOG(LogTeams, Verbose, TEXT("Requested team tag count on an unknown team (TeamId: %d, Tag: %s)."), TeamId, *Tag.ToString());
	}

	return 0;
}

bool UTeamSubsystem::TeamHasTag(int32 TeamId, FGameplayTag Tag) const
{
	// Return if given team has the requested tag, if it exists.
	if (ATeamInfo* TeamInfo = *TeamMap.Find(TeamId))
	{
		return TeamInfo->TeamTags.ContainsTag(Tag);
	}
	// The team cannot be found.
	else if (!TeamMap.Contains(TeamId))
	{
		UE_LOG(LogTeams, Verbose, TEXT("Requested team tag check on an unknown team (TeamId: %d, Tag: %s)."), TeamId, *Tag.ToString());
	}

	return false;
}

ETeamAlignment UTeamSubsystem::CompareTeams(const UObject* A, const UObject* B, int32& TeamA, int32& TeamB) const
{
	TeamA = FindTeamFromObject(A);
	TeamB = FindTeamFromObject(B);

	// Comparison is invalid if at least one object has an invalid team.
	if ((TeamA == INDEX_NONE) || (TeamB == INDEX_NONE))
	{
		return ETeamAlignment::InvalidArgument;
	}
	// Compare teams if both are valid.
	else
	{
		return (TeamA == TeamB) ? ETeamAlignment::SameTeam : ETeamAlignment::DifferentTeams;
	}
}

ETeamAlignment UTeamSubsystem::CompareTeams(const UObject* A, const UObject* B) const
{
	int32 OutTeamA;
	int32 OutTeamB;
	return CompareTeams(A, B, OutTeamA, OutTeamB);
}

bool UTeamSubsystem::CanCauseDamage(const UObject* Instigator, const UObject* Target, bool bAllowDamageToSelf) const
{
	// If the instigator can damage itself, check if it's trying to. Otherwise, we'll end up returning false.
	if (bAllowDamageToSelf)
	{
		// Check if the instigator is also the target, or has the same player state as the target.
		if ((Instigator == Target) || (FindPlayerStateFromActor(Cast<AActor>(Instigator)) == FindPlayerStateFromActor(Cast<AActor>(Target))))
		{
			return true;
		}
	}

	int32 InstigatorTeam;
	int32 TargetTeam;
	const ETeamAlignment Alignment = CompareTeams(Instigator, Target, InstigatorTeam, TargetTeam);

	// Objects of different teams can damage each other.
	if (Alignment == ETeamAlignment::DifferentTeams)
	{
		return true;
	}
	// If an object has a team, it can damage objects without a team if that object has an ASC.
	else if ((Alignment == ETeamAlignment::InvalidArgument) && (InstigatorTeam != INDEX_NONE))
	{
		return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Cast<const AActor>(Target)) != nullptr;
	}

	return false;
}

bool UTeamSubsystem::CanCauseHealing(const UObject* Instigator, const UObject* Target, bool bAllowHealSelf) const
{
	// If the instigator can NOT heal itself, check if it's trying to. Otherwise, we'll end up returning true.
	if (!bAllowHealSelf)
	{
		// Check if the instigator is also the target, or has the same player state as the target.
		if ((Instigator == Target) || (FindPlayerStateFromActor(Cast<AActor>(Instigator)) == FindPlayerStateFromActor(Cast<AActor>(Target))))
		{
			return false;
		}
	}

	int32 InstigatorTeam;
	int32 TargetTeam;
	const ETeamAlignment Alignment = CompareTeams(Instigator, Target, InstigatorTeam, TargetTeam);

	// Objects of the same team can heal each other.
	if (Alignment == ETeamAlignment::SameTeam)
	{
		return true;
	}

	return false;
}

UTeamDisplayAsset* UTeamSubsystem::GetTeamDisplayAsset(int32 TeamId, int32 ViewerTeamId)
{
	if (TeamMap.Contains(TeamId))
	{
		if (ATeamInfo* TeamInfo = *TeamMap.Find(TeamId))
		{
			// If the viewer is on the same (valid) team as the target, return the "friendly" display asset, if one exists.
			if ((TeamId != INDEX_NONE) && (TeamId == ViewerTeamId))
			{
				if (TeamInfo->GetFriendlyDisplayAsset() != nullptr)
				{
					return TeamInfo->GetFriendlyDisplayAsset();
				}
			}

			/* If the viewer and target are on different teams, or if friendly display assets aren't being used in this
			 * mode, use the team's normal display asset. */
			return TeamInfo->GetTeamDisplayAsset();
		}
	}

	return nullptr;
}

void UTeamSubsystem::NotifyTeamDisplayAssetModified(UTeamDisplayAsset* ModifiedAsset)
{
	// Broadcast TeamDisplayAssetChangedDelegate for any teams using the modified asset.
	for (const auto& KVP : TeamMap)
	{
		if ((KVP.Value->GetTeamDisplayAsset() == ModifiedAsset) || (KVP.Value->GetFriendlyDisplayAsset() == ModifiedAsset))
		{
			KVP.Value->TeamDisplayAssetChangedDelegate.Broadcast(ModifiedAsset);
		}
	}
}

FTeamDisplayAssetChangedSignature* UTeamSubsystem::GetTeamDisplayAssetChangedDelegate(int32 TeamId)
{
	if (TeamMap.Contains(TeamId))
	{
		if (ATeamInfo* TeamInfo = *TeamMap.Find(TeamId))
		{
			return &TeamInfo->TeamDisplayAssetChangedDelegate;
		}
	}

	return nullptr;
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
	return TeamMap.Contains(IntegerToGenericTeamId(TeamId));
}

TArray<int32> UTeamSubsystem::GetTeamIds() const
{
	TArray<int32> Result;
	TeamMap.GenerateKeyArray(Result);
	Result.Sort();
	return Result;
}
