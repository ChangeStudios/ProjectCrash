// Copyright Samuel Reitich. All rights reserved.

#include "ProjectCrashEditor.h"

#include "GameFramework/GameFeatures/GameFeatureManager.h"
#include "Modules/ModuleManager.h" 

DEFINE_LOG_CATEGORY(LogProjectCrashEditor);

/*
 * Default editor module for this project.
 */
class FProjectCrashEditorModule : public FDefaultGameModuleImpl
{
    typedef FProjectCrashEditorModule ThisClass;

	virtual void StartupModule() override
	{
		if (!IsRunningGame())
		{
			// Bind callback to when PIE starts.
			FEditorDelegates::BeginPIE.AddRaw(this, &ThisClass::OnBeginPIE);
		}
	}

	void OnBeginPIE(bool bIsSimulating)
	{
		// Notify the game feature manager subsystem when a PIE session starts.
		UGameFeatureManager* GameFeatureManager = GEngine->GetEngineSubsystem<UGameFeatureManager>();
		check(GameFeatureManager);
		GameFeatureManager->OnPlayInEditorBegin();
	}

	virtual void ShutdownModule() override
    {
		// Unbind callbacks from when PIE starts.
	    FEditorDelegates::BeginPIE.RemoveAll(this);
    }
};

IMPLEMENT_MODULE(FProjectCrashEditorModule, ProjectCrashEditor);