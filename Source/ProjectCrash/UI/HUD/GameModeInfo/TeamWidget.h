// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "GameFramework/Teams/CrashTeams.h"
#include "TeamWidget.generated.h"

class ACrashPlayerState;
class UBorder;

/**
 * Provides information about the bound team to this widget's owning player, such as the team members' health and life.
 */
UCLASS(Abstract)
class PROJECTCRASH_API UTeamWidget : public UCommonUserWidget
{
	GENERATED_BODY()

	// Player.

public:

	/** Binds this widget's information to the given player. */
	void BindPlayer(APlayerState* InPlayer);

protected:

	/** The players to which this widget is currently bound. */
	UPROPERTY(BlueprintReadOnly)
	TArray<TObjectPtr<ACrashPlayerState>> BoundPlayers;

	/** The team to which this widget is currently bound. */
	FCrashTeamID BoundTeam;



	// Widgets.

protected:

	/** Background widget displaying the bound player's team color. */
	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UBorder> PlayerBox_Border;
};
