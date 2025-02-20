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

	// Developer settings.

public:

	/** Whether game logic should to skip straight to the gameplay phase. Always returns false, except during PIE
	 * sessions when bSkipToGameplay is enabled in developer settings. */
	UFUNCTION(BlueprintCallable, Category = "Development")
	static bool ShouldSkipDirectlyToGameplay();



	// Utils.

public:

	/** Finds the optimal world for running server cheats in PIE. */
	static UWorld* FindPlayInEditorAuthorityWorld();

	/**
	 * Attempts to find a class with a given name.
	 *
	 * For blueprint classes, the name should be formatted as "[Name].[Name]_C".
	 */
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
