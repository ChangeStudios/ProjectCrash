// Copyright Samuel Reitich. All rights reserved.


#include "Development/CrashDeveloperSettings.h"

#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

#define LOCTEXT_NAMESPACE "CrashDeveloperSettings"

UCrashDeveloperSettings::UCrashDeveloperSettings()
{
}

FName UCrashDeveloperSettings::GetCategoryName() const
{
	// Use the project's name as the category of these developer settings.
	return FApp::GetProjectName();
}

#if WITH_EDITOR

void UCrashDeveloperSettings::OnPlayInEditorBegin() const
{
	// Notify user of an active game mode override.
	if (GameModeDataOverride.IsValid())
	{
		FNotificationInfo Info(FText::Format(
			LOCTEXT("GameModeDataOverrideActive", "Developer Settings Override\nGame Mode Data: {0}"),
			FText::FromName(GameModeDataOverride.PrimaryAssetName)
		));
		Info.ExpireDuration = 5.0f;
		FSlateNotificationManager::Get().AddNotification(Info);
	}
	
	// Notify user of an active Challenger override.
	if (ChallengerDataOverride.IsValid())
	{
		FNotificationInfo Info(FText::Format(
			LOCTEXT("ChallengerDataOverrideActive", "Developer Settings Override\nChallenger Data: {0}"),
			FText::FromName(ChallengerDataOverride.PrimaryAssetName)
		));
		Info.ExpireDuration = 5.0f;
		FSlateNotificationManager::Get().AddNotification(Info);
	}

	// Notify user of an active skin override.
	if (SkinDataOverride.IsValid())
	{
		FNotificationInfo Info(FText::Format(
			LOCTEXT("SkinDataOverrideActive", "Developer Settings Override\nSkin Data: {0}"),
			FText::FromName(SkinDataOverride.PrimaryAssetName)
		));
		Info.ExpireDuration = 5.0f;
		FSlateNotificationManager::Get().AddNotification(Info);
	}
}

#endif // WITH_EDITOR

#undef LOCTEXT_NAMESPACE