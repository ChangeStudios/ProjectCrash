// Copyright Samuel Reitich 2024.


#include "UI/HUD/GameModeInfo/GameModeInfoBase.h"

#include "CommonNumericTextBlock.h"
#include "TeamWidget.h"
#include "Components/DynamicEntryBox.h"
#include "GameFramework/GameModes/Game/CrashGameMode.h"
#include "GameFramework/GameStates/CrashGameState.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerState.h"
#include "Player/PlayerStates/CrashPlayerState.h"

void UGameModeInfoBase::NativeConstruct()
{
	Super::NativeConstruct();

	// Cache the game state.
	AGameStateBase* GS = UGameplayStatics::GetGameState(GetOwningPlayer());
	CrashGS = GS ? Cast<ACrashGameState>(GS) : nullptr;

	if (CrashGS)
	{
		// Bind the match timer text.
		CrashGS->PhaseTimeChangedDelegate.AddUniqueDynamic(this, &ThisClass::OnTimeChanged);

		// Register for match state updates.
		CrashGS->MatchStateChangedDelegate.AddUniqueDynamic(this, &ThisClass::OnMatchStateChanged);
	}
}

void UGameModeInfoBase::NativeDestruct()
{
	Super::NativeDestruct();

	// Clear callbacks.
	if (CrashGS)
	{
		CrashGS->PhaseTimeChangedDelegate.RemoveDynamic(this, &ThisClass::OnTimeChanged);
		CrashGS->MatchStateChangedDelegate.RemoveDynamic(this, &ThisClass::OnMatchStateChanged);
	}
}

void UGameModeInfoBase::OnTimeChanged(uint32 NewTime)
{
	// Update the timer text.
	if (Timer_Text)
	{
		Timer_Text->SetCurrentValue(NewTime);
	}
}

void UGameModeInfoBase::OnMatchStateChanged(FName NewMatchState)
{
	// Play an animation when overtime begins.
	if (NewMatchState == CrashMatchState::InProgress_OT && OvertimeStart_Anim)
	{
		PlayAnimation(OvertimeStart_Anim);
		return;
	}

	// Play an animation when overtime ends, to remove the "overtime" pop-up.
	if (NewMatchState != CrashMatchState::InProgress_OT && OvertimeEnd_Anim)
	{
		PlayAnimation(OvertimeEnd_Anim);
		return;
	}
}

void UGameModeInfoBase::CreatePlayerWidget(APlayerState* TargetPlayer)
{
	APlayerState* LocalPS = GetOwningLocalPlayer()->PlayerController->PlayerState;
	UDynamicEntryBox* TargetEntryBox = nullptr;

	// Choose which entry box to create the new player widget in.
	switch (FCrashTeamID::GetAttitude(TargetPlayer, LocalPS))
	{
		case Friendly:
		{
			TargetEntryBox = TeammateWidgets_EntryBox;
			break;
		}
		case Hostile:
		{
			TargetEntryBox = EnemyWidgets_EntryBox;
			break;
		}
		default:
		{
		}
	}

	// Create and bind a new widget for the player.
	if (TargetEntryBox)
	{
		UTeamWidget* TeamWidget = TargetEntryBox->CreateEntry<UTeamWidget>();
		TeamWidget->BindPlayer(TargetPlayer);
		PlayerWidgets.Add(TargetPlayer, TeamWidget);
	}
}

void UGameModeInfoBase::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Check if there are any players in the match that the owning player hasn't created widgets for yet.
	if (CrashGS)
	{
		for (APlayerState* PS : CrashGS->PlayerArray)
		{
			if (!PlayerWidgets.Contains(PS))
			{
				CreatePlayerWidget(PS);
			}
		}
	}
}

