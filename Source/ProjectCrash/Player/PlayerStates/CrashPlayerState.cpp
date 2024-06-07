// Copyright Samuel Reitich 2024.


#include "Player/PlayerStates/CrashPlayerState.h"

#include "CrashGameplayTags.h"
#include "AbilitySystem/AttributeSets/HealthAttributeSet.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "GameFramework/GameModes/Data/CrashGameModeData.h"
#include "GameFramework/GameModes/Game/CrashGameMode.h"
#include "GameFramework/GameStates/CrashGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Player/PlayerControllers/CrashPlayerController.h"
#include "UI/Data/MatchUserInterfaceData.h"

ACrashPlayerState::ACrashPlayerState(const FObjectInitializer& ObjectInitializer)
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

#if WITH_EDITOR
	CurrentChallenger = DefaultChallenger;
	CurrentSkin = DefaultSkin;
#endif	
}

void ACrashPlayerState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Initialize the ASC's actor info with this player state as its owner.
	check(AbilitySystemComponent);
	AbilitySystemComponent->InitAbilityActorInfo(this, GetPawn());

	// Bind the OnInputBlockingChanged callback to when this player's ASC gains or loses the InputBlocking tag.
	InputBlockingDelegate = AbilitySystemComponent->RegisterGameplayTagEvent(CrashGameplayTags::TAG_Ability_Behavior_InputBlocking, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ACrashPlayerState::OnInputBlockingChanged);


	/* Initialize this player's lives using the game mode's data. This is only done on the server (the game mode only
	 * exists on the server) and then replicated to clients. */
	if (HasAuthority())
	{
		/* We may want to change this to use the game state for consistency; the game mode data will be valid on both
		 * the game mode and the game state on the server. */
		const AGameModeBase* GM = UGameplayStatics::GetGameMode(this);
		const ACrashGameMode* CrashGM = GM ? Cast<ACrashGameMode>(GM) : nullptr;
		if (CrashGM && CrashGM->GetGameModeData())
		{
			const UCrashGameModeData* GameModeData = CrashGM->GetGameModeData();
			CurrentLives = GameModeData->StartingLives;
		}
		else
		{
			const uint8 StartingLivesFallback = 1;
			CurrentLives = StartingLivesFallback;

			UE_LOG(LogGameMode, Warning, TEXT("ACrashPlayerState: CrashPlayerState [%s] tried to initialize its current lives, but could not find a game mode with valid GameModeData. ACrashPlayerState must be used with ACrashGameMode, and the game mode must have valid game mode data. Falling back to default starting lives: %i."), *GetName(), StartingLivesFallback);
		}

// Initialize Challenger and Skin from editor instead of game mode.
#if WITH_EDITOR
		UpdateCurrentChallenger(DefaultChallenger);
		UpdateCurrentSkin(DefaultSkin);
#endif // WITH_EDITOR
	}
}

void ACrashPlayerState::Client_HandleMatchEnded_Implementation(bool bWon)
{
	const AGameStateBase* GS = UGameplayStatics::GetGameState(this);
	const ACrashGameState* CrashGS = GS ? Cast<ACrashGameState>(GS) : nullptr;
	const UCrashGameModeData* GMData = CrashGS ? CrashGS->GetGameModeData() : nullptr;
	const UUserInterfaceData* UIData = GMData ? GMData->UIData : nullptr;
	const UMatchUserInterfaceData* MatchUIData = UIData ? Cast<UMatchUserInterfaceData>(UIData) : nullptr;

	APlayerController* PC = GetPlayerController();
	ACrashPlayerController* CrashPC = PC ? Cast<ACrashPlayerController>(PC) : nullptr;

	// Push the "victory"/"defeat" pop-up to the player.
	if (MatchUIData && CrashPC)
	{
		CrashPC->PushWidgetToLayer(bWon ? MatchUIData->VictoryPopUp : MatchUIData->DefeatPopUp, CrashGameplayTags::TAG_UI_Layer_GameMenu);
	}
}

void ACrashPlayerState::Client_HandleLeavingMap_Implementation()
{
	// Return to the main menu when the post-match phase ends.
	// TODO: Use data to find the main menu map.
	UGameplayStatics::OpenLevel(this, FName("L_MenuBackground_Fighters"));
}

void ACrashPlayerState::SetTeamID(FCrashTeamID InTeamID)
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

void ACrashPlayerState::OnRep_TeamID(FCrashTeamID OldTeamID)
{
}

void ACrashPlayerState::DecrementLives_Implementation()
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

void ACrashPlayerState::OnRep_CurrentLives(uint8 OldValue)
{
	// Broadcast the change in current lives.
	LivesChangedDelegate.Broadcast(this, OldValue, CurrentLives);
}

void ACrashPlayerState::UpdateCurrentChallenger(UChallengerData* InChallengerData)
{
	// Only update the challenger on the server.
	if (HasAuthority() && InChallengerData != nullptr)
	{
		CurrentChallenger = InChallengerData;
	}
}

void ACrashPlayerState::UpdateCurrentSkin(UChallengerSkinData* InSkinData)
{
	// Only update the character skin on the server.
	if (HasAuthority() && InSkinData)
	{
		// TODO: Perform additional API validation with player cloud data.

		CurrentSkin = InSkinData;
	}
}

UAbilitySystemComponent* ACrashPlayerState::GetAbilitySystemComponent() const
{
	// The interfaced accessor will always return the typed ASC.
	return GetCrashAbilitySystemComponent();
}

void ACrashPlayerState::OnInputBlockingChanged(const FGameplayTag Tag, int32 NewCount)
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

void ACrashPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(ACrashPlayerState, TeamID, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ACrashPlayerState, CurrentLives, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME(ACrashPlayerState, CurrentChallenger);
	DOREPLIFETIME(ACrashPlayerState, CurrentSkin);
}