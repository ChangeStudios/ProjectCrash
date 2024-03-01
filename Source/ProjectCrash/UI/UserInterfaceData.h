// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UserInterfaceData.generated.h"

class UCommonActivatableWidget;
class ULayeredBaseWidget;

/**
 * The layers of the user interface to which a UI widget can belong. 
 */
UENUM(BlueprintType)
enum EUserInterfaceLayer : uint8
{
	// The in-game HUD. Displays health, abilities, game mode data, etc.
	Game,
	// Any menus that are brought up in-game, such as the pause menu and its various sub-menus.
	GameMenu,
	// UI serving as the primary element of the game outside of gameplay, such as the main menu.
	Menu
};

/**
 * A global collection of data defining the user interface.
 */
UCLASS(Blueprintable, BlueprintType)
class PROJECTCRASH_API UUserInterfaceData : public UDataAsset
{
	GENERATED_BODY()

public:

	/** The base widget that will be created and pushed when the game starts. All other widgets will be pushed to this
	 * widget. */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ULayeredBaseWidget> BaseWidget;
};
