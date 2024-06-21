// Copyright Samuel Reitich. All rights reserved.


#include "ProjectCrashEditorEngine.h"

#include "Development/CrashDeveloperSettings.h"
#include "Framework/Notifications/NotificationManager.h"
#include "GameFramework/CrashWorldSettings.h"
#include "Settings/ContentBrowserSettings.h"
#include "Widgets/Notifications/SNotificationList.h"

#define LOCTEXT_NAMESPACE "ProjectCrashEditor"

void UProjectCrashEditorEngine::Init(IEngineLoop* InEngineLoop)
{
	Super::Init(InEngineLoop);

	// Force showing plugin content, since this project uses game feature plugins for core functionality.
	GetMutableDefault<UContentBrowserSettings>()->SetDisplayPluginFolders(true);
}

FGameInstancePIEResult UProjectCrashEditorEngine::PreCreatePIEInstances(const bool bAnyBlueprintErrors, const bool bStartInSpectatorMode, const float PIEStartTime, const bool bSupportsOnlinePIE, int32& InNumOnlinePIEInstances)
{
	if (const ACrashWorldSettings* CrashWorldSettings = Cast<ACrashWorldSettings>(EditorWorld->GetWorldSettings()))
	{
		if (CrashWorldSettings->ForceStandaloneNetMode)
		{
			EPlayNetMode OutPlayNetMode;
			PlaySessionRequest->EditorPlaySettings->GetPlayNetMode(OutPlayNetMode);

			if (OutPlayNetMode != PIE_Standalone)
			{
				// Override the net mode if it doesn't match the desired Standalone.
				PlaySessionRequest->EditorPlaySettings->SetPlayNetMode(PIE_Standalone);

				// Push a notification informing the user of the overriding net mode.
				FNotificationInfo Info(LOCTEXT("ForcingStandaloneForFrontEnd", "Forcing NetMode: Standalone for front end."));
				Info.ExpireDuration = 5.0f;
				FSlateNotificationManager::Get().AddNotification(Info);
			}
		}
	}

	// Notify developer settings that PIE began, so it can push notifications for the other setting overrides.
	GetDefault<UCrashDeveloperSettings>()->OnPlayInEditorBegin();

	return Super::PreCreatePIEInstances(bAnyBlueprintErrors, bStartInSpectatorMode, PIEStartTime, bSupportsOnlinePIE, InNumOnlinePIEInstances);
}

#undef LOCTEXT_NAMESPACE