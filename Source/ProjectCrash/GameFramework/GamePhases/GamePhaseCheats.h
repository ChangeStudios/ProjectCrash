// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "GamePhaseCheats.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTCRASH_API UGamePhaseCheats : public UCheatManagerExtension
{
	GENERATED_BODY()
	
public:

	UFUNCTION(Exec)
	void StartPhase(const FString& PhaseName);
};
