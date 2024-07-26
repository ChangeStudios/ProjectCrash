// Copyright Samuel Reitich. All rights reserved.


#include "GameFramework/Teams/TeamCreationComponent.h"

#include "TeamInfo.h"
#include "TeamSubsystem.h"
#include "GameFramework/CrashLogging.h"
#include "GameFramework/GameModes/CrashGameMode.h"
#include "GameFramework/GameModes/GameModeManagerComponent.h"
#include "Player/CrashPlayerState.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#define LOCTEXT_NAMESPACE "TeamCreationComponent"

void UTeamCreationComponent::BeginPlay()
{
	Super::BeginPlay();

	// Register to create teams when the game mode loads.
	AGameStateBase* GameState = GetGameStateChecked<AGameStateBase>();
	UGameModeManagerComponent* GameModeManagerComponent = GameState->FindComponentByClass<UGameModeManagerComponent>();
	check(GameModeManagerComponent);
	GameModeManagerComponent->CallOrRegister_OnGameModeLoaded(FCrashGameModeLoadedSignature::FDelegate::CreateUObject(this, &ThisClass::OnGameModeLoaded), ECrashGameModeLoadedResponsePriority::First);
}

void UTeamCreationComponent::OnGameModeLoaded(const UCrashGameModeData* GameModeData)
{
#if WITH_SERVER_CODE
	// Create and assign teams when the game mode loads.
	if (HasAuthority())
	{
		CreateTeams();
		AssignPlayersToTeams();
	}
#endif // WITH_SERVER_CODE
}

#if WITH_SERVER_CODE
void UTeamCreationComponent::CreateTeams()
{
	// By default, just create a unique team for each team defined in this component's map.
	for (const auto &KVP : TeamsToCreate)
	{
		const int32 TeamId = KVP.Key;
		CreateTeam(TeamId, KVP.Value);
	}
}

void UTeamCreationComponent::AssignPlayersToTeams()
{
	// Assign teams to players that have already connected.
	AGameStateBase* GameState = GetGameStateChecked<AGameStateBase>();
	for (APlayerState* PS : GameState->PlayerArray)
	{
		if (ACrashPlayerState* CrashPS = Cast<ACrashPlayerState>(PS))
		{
			ChooseTeamForPlayer(CrashPS);
		}
	}

	// Listen for any new players, to assign them a team when they join.
	ACrashGameMode* GameMode = Cast<ACrashGameMode>(GameState->AuthorityGameMode);
	check(GameMode);
	GameMode->OnGameModePlayerInitializeDelegate.AddUObject(this, &ThisClass::OnPlayerInitialized);
}

void UTeamCreationComponent::ChooseTeamForPlayer(ACrashPlayerState* CrashPS)
{
	// Assign spectators to FGenericTeamId::NoTeam.
	if (CrashPS->IsOnlyASpectator())
	{
		CrashPS->SetGenericTeamId(FGenericTeamId::NoTeam);
		return;
	}

	// TODO: Check game instance for pre-assigned teams from lobbies (map of unique net IDs to team IDs).

	CrashPS->SetGenericTeamId(IntegerToGenericTeamId(GetLeastPopulatedTeam()));
}

void UTeamCreationComponent::OnPlayerInitialized(AGameModeBase* GameMode, AController* NewPlayer)
{
	check(NewPlayer);
	check(NewPlayer->PlayerState);

	// Assign new players a team when they join.
	if (ACrashPlayerState* CrashPS = Cast<ACrashPlayerState>(NewPlayer->PlayerState))
	{
		ChooseTeamForPlayer(CrashPS);
	}
}

void UTeamCreationComponent::CreateTeam(int32 TeamId, UTeamDisplayAsset* DisplayAsset)
{
	check(HasAuthority());

	UWorld* World = GetWorld();
	check(World);

	// Make sure the team doesn't already exist.
	UTeamSubsystem* TeamSubsystem = World->GetSubsystem<UTeamSubsystem>();
	check(TeamSubsystem);
	if (TeamSubsystem->DoesTeamExist(TeamId))
	{
		UE_LOG(LogTeams, Warning, TEXT("Tried to create team %d, but a team with a matching ID already exists."), TeamId);
		return;
	}

	// Spawn a new team info actor for the new team.
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ATeamInfo* TeamInfo = World->SpawnActor<ATeamInfo>(ATeamInfo::StaticClass(), SpawnInfo);
	checkf(TeamInfo != nullptr, TEXT("Failed to create team info actor for team %d."), TeamId);

	// Initialize the new team's data.
	TeamInfo->SetTeamId(TeamId);
	TeamInfo->SetTeamDisplayAsset(DisplayAsset);

	if (bUseFriendlyDisplayAsset)
	{
		TeamInfo->SetFriendlyDisplayAsset(FriendlyDisplayAsset);
	}
}

int32 UTeamCreationComponent::GetLeastPopulatedTeam() const
{
	if (const int32 NumTeams = TeamsToCreate.Num())
	{
		// Initialize a team member counter.
		TMap<uint32, uint32> TeamMemberCounts;
		TeamMemberCounts.Reserve(NumTeams);

		for (const auto& KVP : TeamsToCreate)
		{
			const int32 TeamId = KVP.Key;
			TeamMemberCounts.Add(TeamId, 0);
		}

		// Count the number of players on each team.
		AGameStateBase* GameState = GetGameStateChecked<AGameStateBase>();
		for (APlayerState* PS : GameState->PlayerArray)
		{
			if (ACrashPlayerState* CrashPS = Cast<ACrashPlayerState>(PS))
			{
				const int32 PlayerTeamId = CrashPS->GetTeamId();

				// Don't count unassigned or inactive players.
				if ((PlayerTeamId != INDEX_NONE) && !CrashPS->IsInactive())
				{
					check(TeamMemberCounts.Contains(PlayerTeamId));
					TeamMemberCounts[PlayerTeamId] += 1;
				}
			}
		}

		// Sort by lowest team population, then by team ID.
		int32 BestTeam = INDEX_NONE;
		uint32 BestPlayerCount = TNumericLimits<uint32>::Max();

		for (const auto& KVP : TeamMemberCounts)
		{
			const int32 ItTeamId = KVP.Key;
			const uint32 ItTeamPlayerCount = KVP.Value;

			// Check for lower player count.
			if (ItTeamPlayerCount < BestPlayerCount)
			{
				BestTeam = ItTeamId;
				BestPlayerCount = ItTeamPlayerCount;
			}
			// Check for same player count, but lower team ID.
			else if (ItTeamPlayerCount == BestPlayerCount)
			{
				if ((ItTeamId < BestTeam) || (BestTeam == INDEX_NONE))
				{
					BestTeam = ItTeamId;
					BestPlayerCount = ItTeamPlayerCount;
				}
			}	
		}

		return BestTeam;
	}

	return INDEX_NONE;
}
#endif // WITH_SERVER_CODE

#if WITH_EDITOR
EDataValidationResult UTeamCreationComponent::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (bUseFriendlyDisplayAsset)
	{
		// If a friendly display asset is requested, make sure one is set.
		if (FriendlyDisplayAsset == nullptr)
		{
			Context.AddError(FText::Format(LOCTEXT("FriendlyDisplayAssetNotFound", "[%s] requested to use friendly display assets, but the friendly display asset was not set."), FText::FromString(GetPathNameSafe(this))));
			Result = EDataValidationResult::Invalid;
		}
		// Make sure the friendly display asset is not used for any enemy teams.
		else
		{
			TArray<TObjectPtr<UTeamDisplayAsset>> TeamDisplayAssets;
			TeamsToCreate.GenerateValueArray(TeamDisplayAssets);

			if (TeamDisplayAssets.Contains(FriendlyDisplayAsset))
			{
				Context.AddError(FText::Format(LOCTEXT("FriendlyDisplayAssetNotUnique", "[%s] uses a display asset in its team map which is used as the friendly display asset. This will cause cause some enemy teams to appear as friendly to some players. Use a unique display asset as the friendly display asset."), FText::FromString(GetPathNameSafe(this))));
				Result = EDataValidationResult::Invalid;
			}
		}
	}

	// TODO: Validate display assets.
	return Result;
}
#endif // WITH_EDITOR

#undef LOCTEXT_NAMESPACE