// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/AttributeSets/CrashAttributeSet.h"
#include "LivesAttributeSet.generated.h"

/**
 * Handles players' "lives." A player loses a life when they die. When a player loses all of their lives, they can no
 * longer respawn, and usually enter a spectating state.
 *
 * Lives are not used in all game modes. In most game modes that use lives, players lose the game upon running out.
 */
UCLASS()
class PROJECTCRASH_API ULivesAttributeSet : public UCrashAttributeSet
{
	GENERATED_BODY()
};
