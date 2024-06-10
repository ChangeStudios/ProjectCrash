// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonGameInstance.h"
#include "CrashGameInstance.generated.h"

/**
 * Default game instance for this project. Sets up initialization states.
 */
UCLASS()
class PROJECTCRASH_API UCrashGameInstance : public UCommonGameInstance
{
	GENERATED_BODY()

	// Initialization.

public:

	/** Default constructor. */
	UCrashGameInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Loads the local user's settings when they log in. */
	virtual void HandlerUserInitialized(const UCommonUserInfo* UserInfo, bool bSuccess, FText Error, ECommonUserPrivilege RequestedPrivilege, ECommonUserOnlineContext OnlineContext) override;

protected:

	/** Registers initialization states. */
	virtual void Init() override;

};
