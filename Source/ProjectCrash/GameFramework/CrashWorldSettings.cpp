// Copyright Samuel Reitich. All rights reserved.


#include "GameFramework/CrashWorldSettings.h"

#include "CrashLogging.h"
#include "EngineUtils.h"
#include "Data/CrashGameModeData.h"
#include "Engine/AssetManager.h"
#include "GameFramework/PlayerStart.h"
#include "Misc/UObjectToken.h"

#define LOCTEXT_NAMESPACE "CrashSystem"

FPrimaryAssetId ACrashWorldSettings::GetDefaultGameModeData() const
{
	FPrimaryAssetId ReturnId;

	if (!DefaultGameModeData.IsNull())
	{
		// Retrieve the game mode data's asset ID.
		ReturnId = UAssetManager::Get().GetPrimaryAssetIdForPath(DefaultGameModeData.ToSoftObjectPath());

		if (!ReturnId.IsValid())
		{
			UE_LOG(LogCrashGameMode, Error, TEXT("%s.DefaultGameplayExperience is [%s] but that failed to resolve into an asset ID. You may need to add a path to the asset rules in your game feature plugin or project settings."),
				*GetPathNameSafe(this), *DefaultGameModeData.ToString());
		}
	}

	return ReturnId;
}

#if WITH_EDITOR
void ACrashWorldSettings::CheckForErrors()
{
	Super::CheckForErrors();

	// Make sure the correct player start actor is being used (ACrashPlayerStart).
	FMessageLog MapCheck("MapCheck");
	for (TActorIterator<APlayerStart> PlayerStartIt(GetWorld()); PlayerStartIt; ++PlayerStartIt)
	{
		APlayerStart* PlayerStart = *PlayerStartIt;

		if (IsValid(PlayerStart) && PlayerStart->GetClass() == APlayerStart::StaticClass())
		{
			MapCheck.Warning()
				->AddToken(FUObjectToken::Create(PlayerStart))
				->AddToken(FTextToken::Create(FText::FromString("is a standard APlayerStart. Use ACrashPlayerStart instead.")));
		}
	}

	/* Make sure the selected game mode data can be turned into a primary asset ID (i.e. it's not in an unscanned
	 * directory). */
	FMessageLog GameModeDataCheck("GameModeDataCheck");
	if (!DefaultGameModeData.IsNull())
	{
		if (!UAssetManager::Get().GetPrimaryAssetIdForPath(DefaultGameModeData.ToSoftObjectPath()).IsValid())
		{
			GameModeDataCheck.Warning()
				->AddToken(FTextToken::Create(FText::Format(LOCTEXT("WorldSettingsGameModeDataUnscanned", "Game mode data at [{0}] cannot be resolved into an asset ID. You may need to add a path to the asset rules in your game feature plugin or project settings."), FText::FromString(DefaultGameModeData->GetPathName()))));
		}
	}
}
#endif // WITH_EDITOR

#undef LOCTEXT_NAMESPACE