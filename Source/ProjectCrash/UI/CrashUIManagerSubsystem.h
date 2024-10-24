// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameUIManagerSubsystem.h"
#include "CrashUIManagerSubsystem.generated.h"

/**
 * Default UI Manager Subsystem for this project. Currently does nothing, besides defining this project's default UI
 * policy class.
 */
UCLASS()
class PROJECTCRASH_API UCrashUIManagerSubsystem : public UGameUIManagerSubsystem
{
	GENERATED_BODY()

public:

	UCrashUIManagerSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
};
