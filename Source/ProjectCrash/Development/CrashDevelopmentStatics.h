// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CrashDevelopmentStatics.generated.h"

/**
 * Static library for editor functions.
 */
UCLASS()
class PROJECTCRASH_API UCrashDevelopmentStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/** Finds the optimal world for running server cheats in PIE. */
	static UWorld* FindPlayInEditorAuthorityWorld();

	/** Attempts to find a class with a given name. */
	static UClass* FindClassByName(const FString& SearchToken, UClass* DesiredBaseClass);

	/** Templated version of FindClassByName, returning the found class as a subclass of the desired class. */
	template <typename DesiredClass>
	static TSubclassOf<DesiredClass> FindClassByName(const FString& SearchToken)
	{
		return FindClassByName(SearchToken, DesiredClass::StaticClass());
	}

private:

	/** Helper for retrieving every blueprint in the asset registry. */
	static TArray<FAssetData> GetAllBlueprints();

	/** Helper for retrieving a blueprint from the asset registry by name. Use the blueprint's user-facing name when
	 * calling. */
	static UClass* FindBlueprintClass(const FString& TargetNameRaw, UClass* DesiredBaseClass);
};
