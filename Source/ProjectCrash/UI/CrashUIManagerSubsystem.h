// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "GameUIManagerSubsystem.h"
#include "CrashUIManagerSubsystem.generated.h"

/**
 * The Common Game plugin requires an active UI manager subsystem. We don't use Common Game for our user interface
 * framework, so this class does nothing. But since we do use Common Game for our game framework, we still need this
 * class to keep the plugin happy.
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
