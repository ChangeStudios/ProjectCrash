// Copyright Samuel Reitich. All rights reserved.


#include "Player/PlayerStates/CrashPlayerState_DEP.h"

#include "CrashGameplayTags.h"
#include "AbilitySystem/AttributeSets/HealthAttributeSet.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "GameFramework/Data/CrashGameModeData_DEP.h"
#include "GameFramework/GameModes/Game/CrashGameMode_DEP.h"
#include "GameFramework/GameStates/CrashGameStateBase.h"
#include "GameFramework/GameStates/CrashGameState_DEP.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Player/PlayerControllers/CrashPlayerController_DEP.h"
#include "UI/Data/MatchUserInterfaceData.h"

ACrashPlayerState_DEP::ACrashPlayerState_DEP(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Create the ability system component.
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UCrashAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	MinNetUpdateFrequency = 2.0f;
	NetUpdateFrequency = 100.0f; // The ASC Needs to be updated at a high frequency. This is an upper limit since we enable net.UseAdaptiveNetUpdateFrequency in DefaultEngine.ini.

	// Create this player's attribute sets. These must be created in the same class as their ASC.
	HealthSet = CreateDefaultSubobject<UHealthAttributeSet>(TEXT("HealthSet"));

	// Initialize this player's team to No Team.
	TeamID = FCrashTeamID::NO_TEAM;

	// Initialize other properties.
	CurrentChallenger = nullptr;
	CurrentSkin = nullptr;
}

void ACrashPlayerState_DEP::Client_HandleMatchEnded_Implementation(bool bWon)
{
	const AGameStateBase* GS = UGameplayStatics::GetGameState(this);
	const ACrashGameState_DEP* CrashGS = GS ? Cast<ACrashGameState_DEP>(GS) : nullptr;
	const UCrashGameModeData_DEP* GMData = CrashGS ? CrashGS->GetGameModeData() : nullptr;
	const UUserInterfaceData* UIData = GMData ? GMData->UIData : nullptr;
	const UMatchUserInterfaceData* MatchUIData = UIData ? Cast<UMatchUserInterfaceData>(UIData) : nullptr;

	APlayerController* PC = GetPlayerController();
	ACrashPlayerController_DEP* CrashPC = PC ? Cast<ACrashPlayerController_DEP>(PC) : nullptr;

	// Push the "victory"/"defeat" pop-up to the player.
	if (MatchUIData && CrashPC)
	{
		CrashPC->PushWidgetToLayer(bWon ? MatchUIData->VictoryPopUp : MatchUIData->DefeatPopUp, CrashGameplayTags::TAG_UI_Layer_GameMenu);
	}
}

void ACrashPlayerState_DEP::Client_HandleLeavingMap_Implementation()
{
	// Return to the main menu when the post-match phase ends.
	// TODO: Use data to find the main menu map.
	UGameplayStatics::OpenLevel(this, FName("L_MenuBackground_Fighters"));
}

void ACrashPlayerState_DEP::SetTeamID(FCrashTeamID InTeamID)
{
	// Only the server can change a player's team.
	if (HasAuthority())
	{
		TeamID = InTeamID;
	}
	else
	{
		UE_LOG(LogPlayerManagement, Warning, TEXT("ACrashPlayerState: An attempt was made by [%s] to change TeamID without authority."), *GetName());
	}
}

void ACrashPlayerState_DEP::OnRep_TeamID(FCrashTeamID OldTeamID)
{
}

void ACrashPlayerState_DEP::DecrementLives_Implementation()
{
	if (CurrentLives == 0)
	{
		return;
	}

	// Lives should only ever be reduced one-at-a-time.
	CurrentLives -= 1;

	// The OnRep needs to be called manually on the server.
	OnRep_CurrentLives(CurrentLives + 1);
	
	// If we are out of lives, inform the game mode.
	if (CurrentLives == 0)
	{
		// TODO: Implement out-of-lives game mode notification.
	}
}

void ACrashPlayerState_DEP::OnRep_CurrentLives(uint8 OldValue)
{
	// Broadcast the change in current lives.
	LivesChangedDelegate.Broadcast(this, OldValue, CurrentLives);
}

UAbilitySystemComponent* ACrashPlayerState_DEP::GetAbilitySystemComponent() const
{
	// The interfaced accessor will always return the typed ASC.
	return GetCrashAbilitySystemComponent();
}

void ACrashPlayerState_DEP::OnInputBlockingChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (GetPawn() && GetPawn()->IsLocallyControlled())
	{
		if (NewCount > 0)
		{
			GetPlayerController()->ClientIgnoreLookInput(true);
		}
		else
		{
			GetPlayerController()->ClientIgnoreLookInput(false);
		}
	}
}

void ACrashPlayerState_DEP::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(ACrashPlayerState_DEP, TeamID, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ACrashPlayerState_DEP, CurrentLives, COND_None, REPNOTIFY_Always);
}