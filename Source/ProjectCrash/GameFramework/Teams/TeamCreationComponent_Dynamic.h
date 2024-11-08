// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Teams/TeamCreationComponent.h"
#include "TeamCreationComponent_Dynamic.generated.h"

/**
 * Team creation component that dynamically creates new teams as needed, rather than creating a predefined collection
 * of teams when the game starts.
 *
 * TODO: Implement? May not be needed if we can modify the base class to only create teams that will have players.
 */
UCLASS(Abstract, Blueprintable, HideCategories = (ComponentTick, Tags, ComponentReplication, Cooking, Activation, Variable, AssetUserData, Replication, Navigation), Meta = (ShortToolTip = "Defines which teams are created for a game. Creates new teams for players as needed when they join."))
class PROJECTCRASH_API UTeamCreationComponent_Dynamic : public UTeamCreationComponent
{
	GENERATED_BODY()




};
