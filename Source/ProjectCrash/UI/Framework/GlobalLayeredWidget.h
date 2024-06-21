// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "GlobalLayeredWidget.generated.h"

class UCommonActivatableWidgetStack;

/**
 * An activatable widget with widget stacks for multiple user interface layers. This exposes layered UI to C++,
 * allowing us to push widgets to and pop widgets from this widget.
 */
UCLASS(Abstract, Blueprintable)
class PROJECTCRASH_API UGlobalLayeredWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:

	/** Widget layer responsible for the in-game HUD. Displays health, abilities, game mode information, etc. */
	UPROPERTY(BlueprintReadWrite, Meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> GameLayerStack;

	/** Layer that displays any menus brought up in-game, such as the pause menu. Having this additional layer prevents
	 * the underlying game layer from disappearing when an in-game menu is brought up. */
	UPROPERTY(BlueprintReadWrite, Meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> GameMenuStack;

	/** UI that comprises the primary experience of a game mode, rather than gameplay; e.g. the main menu. */
	UPROPERTY(BlueprintReadWrite, Meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> MenuStack;

	/** Stack for dialog prompts, like action confirmations or error messages. */
	UPROPERTY(BlueprintReadWrite, Meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> DialogStack;
};
