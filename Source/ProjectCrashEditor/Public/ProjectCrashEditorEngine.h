// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "Editor/UnrealEdEngine.h"
#include "ProjectCrashEditorEngine.generated.h"

/**
 * Editor engine for this project.
 */
UCLASS()
class UProjectCrashEditorEngine : public UUnrealEdEngine
{
	GENERATED_BODY()
	
protected:

	/** Applies default settings at startup. */
	virtual void Init(IEngineLoop* InEngineLoop) override;

	/** Overrides PIE net mode with developer settings. */
	virtual FGameInstancePIEResult PreCreatePIEInstances(const bool bAnyBlueprintErrors, const bool bStartInSpectatorMode, const float PIEStartTime, const bool bSupportsOnlinePIE, int32& InNumOnlinePIEInstances) override;
};
