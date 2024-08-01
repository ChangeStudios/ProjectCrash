// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/CrashActivatableWidget.h"
#include "GameModeInfoBase.generated.h"

class ACrashGameState;
class UTeamWidget;
class UGameModePlayerWidget;
class UDynamicEntryBox;
class UCommonNumericTextBlock;

/**
 * Base class for the information bar displaying the time left in the match, each player in the match, and any
 * additional game mode-specific information.
 */
UCLASS()
class PROJECTCRASH_API UGameModeInfoBase : public UCrashActivatableWidget
{
	GENERATED_BODY()

	// Construction.

public:

	/** Registers to timer and match state updates. */
	virtual void NativeConstruct() override;

	/** Updates the timer and checks for any players that have joined the match, but don't have widgets yet. */
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;



	// Destruction.

public:

	/** Clears callbacks. */
	virtual void NativeDestruct() override;



	// Match state.

protected:

	/** Triggers state-change animations. */
	UFUNCTION()
	virtual void OnMatchStateChanged(FName NewMatchState);



	// Players.

protected:

	/** Creates a new player widget for the given player, with alignment towards the local player. */
	UFUNCTION()
	void CreatePlayerWidget(APlayerState* TargetPlayer);

	/** Tracks which players the owning player has created widgets for. */
	UPROPERTY()
	TMap<APlayerState*, UTeamWidget*> PlayerWidgets;



	// Widgets.

protected:

	/** Text displaying the timer left in the match. */
	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UCommonNumericTextBlock> Timer_Text;

	/** Entry box for teammates' profiles, which displays their Challenger, health, and remaining lives. */
	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UDynamicEntryBox> TeammateWidgets_EntryBox;

	/** Entry box for opponents' profiles, which displays their Challenger, health, and remaining lives. */
	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UDynamicEntryBox> EnemyWidgets_EntryBox;



	// Animations.
	
protected:

	/** Widget animation played when the match enters overtime. */
	UPROPERTY(Transient, Meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> OvertimeStart_Anim;

	/** Widget animation played when the match finishes overtime (i.e. the match ends). */
	UPROPERTY(Transient, Meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> OvertimeEnd_Anim;



	// Utils.

protected:

	// Game state used for this widget's information, cached for convenience.
	UPROPERTY()
	TObjectPtr<ACrashGameState> CrashGS;
};
