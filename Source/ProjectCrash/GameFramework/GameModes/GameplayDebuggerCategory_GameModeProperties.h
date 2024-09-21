// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"

#if WITH_GAMEPLAY_DEBUGGER
#include "GameplayDebuggerCategory.h"

class PROJECTCRASH_API FGameplayDebuggerCategory_GameModeProperties : public FGameplayDebuggerCategory
{
public:

	FGameplayDebuggerCategory_GameModeProperties();

	virtual void CollectData(APlayerController* OwnerPC, AActor* DebugActor) override;

	virtual void DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext) override;

	static TSharedRef<FGameplayDebuggerCategory> MakeInstance();

protected:

	FNumberFormattingOptions NumberFormattingOptions;

	struct FRepData
	{
		TArray<TTuple<FString /* Property Name */, float /* Property Value */>> GameModeProperties;
		void Serialize(FArchive& Ar);
	};

	FRepData DataPack;
};

#endif // WITH_GAMEPLAY_DEBUGGER