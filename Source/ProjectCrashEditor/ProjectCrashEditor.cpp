// Copyright Samuel Reitich 2024.

#include "ProjectCrashEditor.h"

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
		UE_LOG(LogTemp, Error, TEXT("PIE STARTED"));
	}

	virtual void ShutdownModule() override
    {
		// Unbind callbacks from when PIE starts.
	    FEditorDelegates::BeginPIE.RemoveAll(this);
    }
};

IMPLEMENT_MODULE(FProjectCrashEditorModule, ProjectCrashEditor);