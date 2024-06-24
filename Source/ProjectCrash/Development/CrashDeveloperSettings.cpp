// Copyright Samuel Reitich. All rights reserved.


#include "Development/CrashDeveloperSettings.h"

#include "Characters/Data/PawnData.h"
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
	
	// Notify user of an active pawn override.
	if (PawnDataOverride)
	{
		FNotificationInfo Info(FText::Format(
			LOCTEXT("PawnDataOverrideActive", "Developer Settings Override\nPawn Data: {0}"),
			FText::FromString(GetNameSafe(PawnDataOverride))
		));
		Info.ExpireDuration = 5.0f;
		FSlateNotificationManager::Get().AddNotification(Info);
	}
}

#endif // WITH_EDITOR

#undef LOCTEXT_NAMESPACE