// Copyright Samuel Reitich. All rights reserved.


#include "GameFramework/Teams/TeamCreationComponent.h"

#include "TeamDisplayAsset.h"
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
	// TODO: We should know by now which teams will have players, and only create those.
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
	GameMode->GameModePlayerInitializedDelegate.AddUObject(this, &ThisClass::OnPlayerInitialized);
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
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

	// Make sure every team display asset has identical properties.
	TArray<TObjectPtr<UTeamDisplayAsset>> TeamDisplayAssets;
	TeamsToCreate.GenerateValueArray(TeamDisplayAssets);
	TMap<UTeamDisplayAsset*, FMissingProperties> MissingProps;

	for (UTeamDisplayAsset* TeamDisplayAsset : TeamDisplayAssets)
	{
		/* Check every other display asset to make sure they also have each of this asset's properties. Currently, we
		 * only check color and texture properties. Scalars will always have default values to fall back on, which we
		 * may want to use instead of explicitly setting our own. */
		for (UTeamDisplayAsset* Other : TeamDisplayAssets)
		{
			if (TeamDisplayAsset == Other)
			{
				continue;
			}

			// // Check for missing scalar properties.
			// for (auto ScalarIt = TeamDisplayAsset->Scalars.CreateConstIterator(); ScalarIt; ++ScalarIt)
			// {
			// 	/* If the other display asset is missing this asset's scalar, add the scalar to its list of missing
			// 	 * properties. Add this display asset as a dependency of that property. */
			// 	if (!(Other->Scalars.Contains(ScalarIt.Key())))
			// 	{
			// 		auto& Entry = MissingProps.FindOrAdd(Other).MissingScalars.FindOrAdd(ScalarIt.Key());
			// 		Entry.AddUnique(TeamDisplayAsset);
			// 	}
			// }
			
			// Check for missing color properties.
			for (auto ColorIt = TeamDisplayAsset->Colors.CreateConstIterator(); ColorIt; ++ColorIt)
			{
				/* If the other display asset is missing this asset's color, add the color to its list of missing
				 * properties. Add this display asset as a dependency of that property. */
				if (!(Other->Colors.Contains(ColorIt.Key())))
				{
					auto& Entry = MissingProps.FindOrAdd(Other).MissingColors.FindOrAdd(ColorIt.Key());
					Entry.AddUnique(TeamDisplayAsset);
				}
			}

			// Check for missing texture properties.
			for (auto TextureIt = TeamDisplayAsset->Textures.CreateConstIterator(); TextureIt; ++TextureIt)
			{
				/* If the other display asset is missing this asset's texture, add the texture to its list of missing
				 * properties. Add this display asset as a dependency of that property. */
				if (!(Other->Textures.Contains(TextureIt.Key())))
				{
					auto& Entry = MissingProps.FindOrAdd(Other).MissingTextures.FindOrAdd(TextureIt.Key());
					Entry.AddUnique(TeamDisplayAsset);
				}
			}
		}
	}

	// Log any missing properties as errors.
	for (auto& Entry : MissingProps)
	{
		/* Builds a list of asset names in the format:
		 *		"Used by Team_Blue, Team_Red, and Team_Pink." */
		auto BuildDependencyString = [](TArray<UTeamDisplayAsset*> Dependencies) -> FString
		{
			if (Dependencies.Num() == 0)
			{
				return FString();
			}

			FString DependencyString = "Used by ";

			for (int i = 0; i < Dependencies.Num(); ++i)
			{
				// First entry
				if (i == 0)
				{
					DependencyString.Append(GetNameSafe(Dependencies[i]));

					if (Dependencies.Num() == 1)
					{
						DependencyString.Append(".");
					}
				}
				// Last entry
				else if (i == (Dependencies.Num() - 1))
				{
					if (Dependencies.Num() == 2)
					{
						DependencyString.Append(" and " + GetNameSafe(Dependencies[i]) + ".");
					}
					else
					{
						DependencyString.Append(", and " + GetNameSafe(Dependencies[i]) + ".");
					}
				}
				// Middle entries
				else
				{
					DependencyString.Append(", " + GetNameSafe(Dependencies[i]));
				}
			}
			
			return DependencyString;
		};

		// Missing scalars.
		for (auto& MissingScalar : Entry.Value.MissingScalars)
		{
			FString ScalarDependencyString = BuildDependencyString(MissingScalar.Value);
			Context.AddError(FText::Format(LOCTEXT("MissingScalar", "{0} is missing scalar property {1}. {2}"), FText::FromString(GetNameSafe(Entry.Key)), FText::FromName(MissingScalar.Key), FText::FromString(ScalarDependencyString)));
			Result = EDataValidationResult::Invalid;
		}

		// Missing colors.
		for (auto& MissingColor : Entry.Value.MissingColors)
		{
			FString ColorDependencyString = BuildDependencyString(MissingColor.Value);
			Context.AddError(FText::Format(LOCTEXT("MissingColor", "{0} is missing color property {1}. {2}"), FText::FromString(GetNameSafe(Entry.Key)), FText::FromName(MissingColor.Key), FText::FromString(ColorDependencyString)));
			Result = EDataValidationResult::Invalid;
		}

		// Missing scalars.
		for (auto& MissingTexture : Entry.Value.MissingTextures)
		{
			FString TextureDependencyString = BuildDependencyString(MissingTexture.Value);
			Context.AddError(FText::Format(LOCTEXT("MissingTexture", "{0} is missing texture property {1}. {2}"), FText::FromString(GetNameSafe(Entry.Key)), FText::FromName(MissingTexture.Key), FText::FromString(TextureDependencyString)));
			Result = EDataValidationResult::Invalid;
		}
	}

	return Result;
}
#endif // WITH_EDITOR

#undef LOCTEXT_NAMESPACE