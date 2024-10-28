// Copyright Samuel Reitich. All rights reserved.

#pragma once

#if WITH_GAMEPLAY_DEBUGGER
#include "GameplayDebuggerCategory.h"

class PROJECTCRASH_API FGameplayDebuggerCategory_GamePhases : public FGameplayDebuggerCategory
{

public:

	FGameplayDebuggerCategory_GamePhases();

	virtual void CollectData(APlayerController* OwnerPC, AActor* DebugActor) override;

	virtual void DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext) override;

	static TSharedRef<FGameplayDebuggerCategory> MakeInstance();

protected:

	struct FRepData
	{
		TArray<FString> ActivePhases;
		void Serialize(FArchive& Ar);
	};

	FRepData DataPack;
};

#endif // WITH_GAMEPLAY_DEBUGGER