// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TeamStatics.generated.h"

class UTeamDisplayAsset;

/**
 * Collection of static utilities for the team system.
 */
UCLASS()
class PROJECTCRASH_API UTeamStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/** Attempts to find any team associated with the given object (via its player state, instigator, etc.). Returns
	 * INDEX_NONE if no team could be found, or if the object is explicitly assigned to NoTeam. */
	UFUNCTION(BlueprintCallable, Category = "Teams", Meta = (DefaultToSelf = "Object", Keywords = "get, get team"))
	static void FindTeamFromObject(const UObject* Object, bool& bIsOnTeam, int32& TeamId);

	/** Retrieves the effective display asset of the given team from the specified viewer's perspective. If no viewer
	 * is given, the team's normal display asset is returned. */
	UFUNCTION(BlueprintCallable, Category = "Teams", Meta = (WorldContext = "WorldContextObject"))
	static UTeamDisplayAsset* GetTeamDisplayAsset(const UObject* WorldContextObject, int32 TeamId, UObject* Viewer = nullptr);

	/** Retrieves the specified color property from the given team display asset. Returns FallbackValue if the property
	 * is not present in the display asset. */
	UFUNCTION(BlueprintCallable, Category = "Teams")
	static FLinearColor GetTeamColor(UTeamDisplayAsset* DisplayAsset, FName ColorName, FLinearColor FallbackValue);
};
