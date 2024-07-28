// Copyright Samuel Reitich. All rights reserved.


#include "UI/HUD/GameModeInfo/GameModeInfoBase.h"

#include "CommonNumericTextBlock.h"
#include "TeamWidget.h"
#include "Components/DynamicEntryBox.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/GameModes/CrashGameState.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerState.h"

void UGameModeInfoBase::NativeConstruct()
{
	Super::NativeConstruct();

	// Cache the game state.
	AGameStateBase* GS = UGameplayStatics::GetGameState(GetOwningPlayer());
	CrashGS = GS ? Cast<ACrashGameState>(GS) : nullptr;
}

void UGameModeInfoBase::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (CrashGS)
	{
		// Update the timer text.
		if (Timer_Text)
		{
			// We have to manually format the timer text because UCommonNumericTextBlock::SetCurrentValue lags slightly.
			// const FTimespan CurrentSeconds = FTimespan::FromSeconds(CrashGS->PhaseTimeRemaining);
			// FText::AsTimespan(CurrentSeconds);
			// Timer_Text->SetText(FText::AsTimespan(CurrentSeconds));
		}

		// Check if there are any players in the match that the owning player hasn't created widgets for yet.
		for (APlayerState* PS : CrashGS->PlayerArray)
		{
			if (!PlayerWidgets.Contains(PS))
			{
				CreatePlayerWidget(PS);
			}
		}
	}
}

void UGameModeInfoBase::NativeDestruct()
{
	Super::NativeDestruct();
}

void UGameModeInfoBase::OnMatchStateChanged(FName NewMatchState)
{
	// Play an animation when overtime begins.
	if (NewMatchState == MatchState::InProgress && OvertimeStart_Anim)
	{
		PlayAnimation(OvertimeStart_Anim);
		return;
	}
}

void UGameModeInfoBase::CreatePlayerWidget(APlayerState* TargetPlayer)
{
	APlayerState* LocalPS = GetOwningLocalPlayer()->PlayerController->PlayerState;
	UDynamicEntryBox* TargetEntryBox = nullptr;

	// Choose which entry box to create the new player widget in.
	// switch (FCrashTeamID::GetAttitude(TargetPlayer, LocalPS))
	// {
	// 	case Friendly:
	// 	{
	// 		TargetEntryBox = TeammateWidgets_EntryBox;
	// 		break;
	// 	}
	// 	case Hostile:
	// 	{
	// 		TargetEntryBox = EnemyWidgets_EntryBox;
	// 		break;
	// 	}
	// 	default:
	// 	{
	// 	}
	// }

	// Create and bind a new widget for the player.
	if (TargetEntryBox)
	{
		UTeamWidget* TeamWidget = TargetEntryBox->CreateEntry<UTeamWidget>();
		TeamWidget->BindPlayer(TargetPlayer);
		PlayerWidgets.Add(TargetPlayer, TeamWidget);
	}
}
