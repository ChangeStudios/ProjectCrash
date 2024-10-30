// Copyright Samuel Reitich. All rights reserved.

#include "GameFramework/Teams/TeamSubsystem.h"

#include "AbilitySystemGlobals.h"
#include "CrashGameplayTags.h"
#include "CrashTeamAgentInterface.h"
#include "GenericTeamAgentInterface.h"
#include "TeamCheats.h"
#include "TeamDisplayAsset.h"
#include "TeamInfo.h"
#include "GameFramework/CheatManager.h"
#include "GameFramework/CrashLogging.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/GameModes/CrashGameState.h"
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

	// Cache the team's display assets from its info actor.
	FriendlyDisplayAsset = NewTeamInfo->GetFriendlyDisplayAsset();
	TeamDisplayAsset = NewTeamInfo->GetTeamDisplayAsset();

	// Broadcast any display asset change.
	if ((OldFriendlyDisplayAsset != FriendlyDisplayAsset) || (OldTeamDisplayAsset != TeamDisplayAsset))
	{
		TeamDisplayAssetChangedDelegate.Broadcast(TeamInfo->GetTeamId());
	}
}

void FTeamTrackingInfo::RemoveTeamInfo(ATeamInfo* InfoToRemove)
{
	ensureMsgf(TeamInfo == InfoToRemove, TEXT("Attempted to remove registered team info, but [%s] was given, which is not this tracking info's team info actor."), *GetNameSafe(InfoToRemove));

	TeamInfo = nullptr;
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

	/* Register a listener for when teams' tags change to forward those callbacks to any listeners who want them via
	 * the ObserveTeamTags node. */
	if (UWorld* World = GetWorld())
	{
		UGameplayMessageSubsystem* MessageSystem = UGameInstance::GetSubsystem<UGameplayMessageSubsystem>(World->GetGameInstance());
		if (ensure(MessageSystem))
		{
			TeamTagsListener = MessageSystem->RegisterListener(CrashGameplayTags::TAG_Message_Team_TagChange, this, &UTeamSubsystem::OnTeamTagChanged);
		}
	}
}

void UTeamSubsystem::Deinitialize()
{
	// Stop listening for the cheat manager to be created.
	UCheatManager::UnregisterFromOnCheatManagerCreated(CheatManagerRegistrationHandle);

	// Stop listening for changes to team tags.
	TeamTagsListener.Unregister();

	Super::Deinitialize();
}

bool UTeamSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

bool UTeamSubsystem::RegisterTeam(ATeamInfo* TeamInfo)
{
	if (ensure(TeamInfo))
	{
		// Make sure the team has a valid ID.
		const int32 TeamId = TeamInfo->GetTeamId();
		if (ensure(TeamId != INDEX_NONE))
		{
			/* Register the team by inserting it into its team's entry in the TeamMap, or creating a new entry for it
			 * if needed. */
			FTeamTrackingInfo& Entry = TeamMap.FindOrAdd(TeamId);
			Entry.SetTeamInfo(TeamInfo);

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
			/* Unregister the team by removing its info actor. We don't remove the entry so that new teams don't try to
			 * re-use the unregistered team's ID. */
			if (FTeamTrackingInfo* Entry = TeamMap.Find(TeamId))
			{
				Entry->RemoveTeamInfo(TeamInfo);

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

	if (FTeamTrackingInfo* Entry = TeamMap.Find(TeamId))
	{
		if (ATeamInfo* TeamInfo = Entry->TeamInfo)
		{
			if (TeamInfo->HasAuthority())
			{
				// Add the given number of tags to the specified team.
				TeamInfo->TeamTags.AddTags(Tag, Count);

				// Broadcast a message indicating the team's tags changed.
				// TODO: FIX
				BroadcastTeamTagChange(TeamInfo, Tag);
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

	if (FTeamTrackingInfo* Entry = TeamMap.Find(TeamId))
	{
		if (ATeamInfo* TeamInfo = Entry->TeamInfo)
		{
			if (TeamInfo->HasAuthority())
			{
				// Remove the given number of tags from the specified team.
				TeamInfo->TeamTags.RemoveTags(Tag, Count);

				// Broadcast a message indicating the team's tags changed.
				BroadcastTeamTagChange(TeamInfo, Tag);
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
	if (const FTeamTrackingInfo* Entry = TeamMap.Find(TeamId))
	{
		return (Entry->TeamInfo ? Entry->TeamInfo->TeamTags.GetTagCount(Tag) : 0);
	}
	// The team cannot be found.
	else
	{
		UE_LOG(LogTeams, Verbose, TEXT("Requested team tag count on an unknown team (TeamId: %d, Tag: %s)."), TeamId, *Tag.ToString());
	}

	return 0;
}

bool UTeamSubsystem::TeamHasTag(int32 TeamId, FGameplayTag Tag) const
{
	// Return if given team has the requested tag, if it exists.
	if (const FTeamTrackingInfo* Entry = TeamMap.Find(TeamId))
	{
		return (Entry->TeamInfo ? Entry->TeamInfo->TeamTags.ContainsTag(Tag) : false);
	}
	// The team cannot be found.
	else
	{
		UE_LOG(LogTeams, Verbose, TEXT("Requested team tag check on an unknown team (TeamId: %d, Tag: %s)."), TeamId, *Tag.ToString());
	}

	return false;
}

void UTeamSubsystem::ObserveTeamTags(int32 TeamId, FGameplayTag Tag, bool bPartialMatch, const FTeamTagChangedSignature& OnTagChanged)
{
	// Register a new listener to fire the given callback when the specified tag changes for the specified team.
	FTeamTagListener Listener;
	Listener.TeamTag = Tag;
	Listener.bPartialMatch = bPartialMatch;
	Listener.TagChangeCallback = OnTagChanged;

	TeamTagListeners.FindOrAdd(TeamId).Add(Listener);
}

void UTeamSubsystem::BroadcastTeamTagChange(ATeamInfo* TeamInfo, FGameplayTag Tag)
{
	// Tag changes are server-authoritative.
	check(TeamInfo->HasAuthority());

	// Broadcast the tag change message locally.
	FCrashTeamTagChangedMessage Message;
	Message.Tag = Tag;
	Message.Count = TeamInfo->TeamTags.GetTagCount(Tag);
	Message.TeamId = TeamInfo->GetTeamId();
	UGameplayMessageSubsystem::Get(this).BroadcastMessage(CrashGameplayTags::TAG_Message_Team_TagChange, Message);

	// Broadcast the message to clients.
	TeamInfo->Multicast_BroadcastTagChange(Message);
}

void UTeamSubsystem::OnTeamTagChanged(FGameplayTag Channel, const FCrashTeamTagChangedMessage& Message)
{
	// Fire each callback registered to the changed tag on the changed team.
	if (TeamTagListeners.Contains(Message.TeamId))
	{
		for (FTeamTagListener Listener : *TeamTagListeners.Find(Message.TeamId))
		{
			if (Listener.IsMatch(Message.Tag))
			{
				Listener.TagChangeCallback.ExecuteIfBound(Message.Count);
			}
		}
	}
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
	if (FTeamTrackingInfo* Entry = TeamMap.Find(TeamId))
	{
		// If the viewer is on the same (valid) team as the target, return the "friendly" display asset, if one exists.
		if ((TeamId != INDEX_NONE) && (TeamId == ViewerTeamId) && Entry->FriendlyDisplayAsset)
		{
			return Entry->FriendlyDisplayAsset;
		}

		/* If the viewer and target are on different teams, or we aren't using friendly display assets, use the team's
		 * normal display asset. */
		return Entry->TeamDisplayAsset;
	}

	return nullptr;
}

void UTeamSubsystem::NotifyTeamDisplayAssetModified(UTeamDisplayAsset* ModifiedAsset)
{
	// Broadcast the display asset change delegate for any teams using the modified asset.
	for (const auto& KVP : TeamMap)
	{
		if ((KVP.Value.FriendlyDisplayAsset == ModifiedAsset) || (KVP.Value.TeamDisplayAsset == ModifiedAsset))
		{
			KVP.Value.TeamDisplayAssetChangedDelegate.Broadcast(KVP.Key);
		}
	}
}

FTeamDisplayAssetChangedSignature& UTeamSubsystem::GetTeamDisplayAssetChangedDelegate(int32 TeamId)
{
	/* Add the given team if it does not exist yet to allow listeners to register for team display asset changes before
	 * the teams finish replicating. */
	FTeamTrackingInfo& Entry = TeamMap.FindOrAdd(TeamId);

	return Entry.TeamDisplayAssetChangedDelegate;
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
