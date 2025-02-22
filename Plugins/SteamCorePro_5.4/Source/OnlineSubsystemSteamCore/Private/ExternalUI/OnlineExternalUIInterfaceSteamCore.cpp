/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#include "ExternalUI/OnlineExternalUIInterfaceSteamCore.h"
#include "OnlineExternalUITypesSteamCore.h"
#include "OnlineSubsystemSteamCorePrivatePCH.h" 

#if WITH_STEAMCORE
FString FOnlineAsyncEventSteamCoreExternalUITriggered::ToString() const
{
	return FString::Printf(TEXT("FOnlineAsyncEventSteamCoreExternalUITriggered bIsActive: %d"), m_bIsActive);
}

void FOnlineAsyncEventSteamCoreExternalUITriggered::TriggerDelegates()
{
	LogSteamCoreVerbose("");
	FOnlineAsyncEvent::TriggerDelegates();
	const IOnlineExternalUIPtr ExternalUIInterface = Subsystem->GetExternalUIInterface();
	ExternalUIInterface->TriggerOnExternalUIChangeDelegates(m_bIsActive);

	if (!m_bIsActive)
	{
		const FOnlineExternalUISteamCorePtr ExternalUISteam = StaticCastSharedPtr<FOnlineExternalUISteamCore>(ExternalUIInterface);
		ExternalUISteam->m_ProfileUIClosedDelegate.ExecuteIfBound();
		ExternalUISteam->m_ProfileUIClosedDelegate.Unbind();

		ExternalUISteam->m_ShowMessageClosedDelegate.ExecuteIfBound(ExternalUISteam->m_bMessageSent);
		ExternalUISteam->m_ShowMessageClosedDelegate.Unbind();
		ExternalUISteam->m_bMessageSent = false;

		ExternalUISteam->m_ShowStoreClosedDelegate.ExecuteIfBound(true);
		ExternalUISteam->m_ShowStoreClosedDelegate.Unbind();

		ExternalUISteam->m_ShowWebUrlClosedDelegate.ExecuteIfBound(TEXT(""));
		ExternalUISteam->m_ShowWebUrlClosedDelegate.Unbind();
	}
}

bool FOnlineExternalUISteamCore::ShowLoginUI(const int ControllerIndex, bool bShowOnlineOnly, bool bShowSkipButton, const FOnLoginUIClosedDelegate& Delegate)
{
	LogSteamCoreVerbose("");
	return false;
}

bool FOnlineExternalUISteamCore::ShowAccountCreationUI(const int ControllerIndex, const FOnAccountCreationUIClosedDelegate& Delegate)
{
	LogSteamCoreVerbose("");
	return false;
}

bool FOnlineExternalUISteamCore::ShowFriendsUI(int32 LocalUserNum)
{
	LogSteamCoreVerbose("");
	SteamFriends()->ActivateGameOverlay("Friends");
	return true;
}

bool FOnlineExternalUISteamCore::ShowInviteUI(int32 LocalUserNum, FName SessionName)
{
	LogSteamCoreVerbose("");
	const IOnlineSessionPtr SessionInt = m_SteamSubsystem->GetSessionInterface();
	if (!SessionInt.IsValid())
	{
		return false;
	}

	const FNamedOnlineSession* const Session = SessionInt->GetNamedSession(SessionName);
	if (Session && Session->SessionInfo.IsValid())
	{
		const FOnlineSessionInfoSteamCore* const SessionInfo = static_cast<FOnlineSessionInfoSteamCore*>(Session->SessionInfo.Get());
		if (SessionInfo->m_SessionType == ESteamSession::LobbySession && SessionInfo->m_SessionId->IsValid())
		{
			SteamFriends()->ActivateGameOverlayInviteDialog(*SessionInfo->m_SessionId);
		}
		else if (SessionInfo->m_SessionType == ESteamSession::AdvertisedSessionHost || SessionInfo->m_SessionType == ESteamSession::AdvertisedSessionClient)
		{
			SteamFriends()->ActivateGameOverlay("LobbyInvite");
		}
		return true;
	}

	return false;
}

bool FOnlineExternalUISteamCore::ShowAchievementsUI(int32 LocalUserNum)
{
	LogSteamCoreVerbose("");
	SteamFriends()->ActivateGameOverlay("Achievements");
	return true;
}

bool FOnlineExternalUISteamCore::ShowLeaderboardUI(const FString& LeaderboardName)
{
	LogSteamCoreVerbose("");
	return false;
}

bool FOnlineExternalUISteamCore::ShowWebURL(const FString& Url, const FShowWebUrlParams& ShowParams, const FOnShowWebUrlClosedDelegate& Delegate)
{
	LogSteamCoreVerbose("");
	if (!Url.StartsWith(TEXT("https://")))
	{
		SteamFriends()->ActivateGameOverlayToWebPage(TCHAR_TO_UTF8(*FString::Printf(TEXT("https://%s"), *Url)));
	}
	else
	{
		SteamFriends()->ActivateGameOverlayToWebPage(TCHAR_TO_UTF8(*Url));
	}

	m_ShowWebUrlClosedDelegate = Delegate;
	return true;
}

bool FOnlineExternalUISteamCore::CloseWebURL()
{
	LogSteamCoreVerbose("");
	return false;
}

bool FOnlineExternalUISteamCore::ShowProfileUI(const FUniqueNetId& Requestor, const FUniqueNetId& Requestee, const FOnProfileUIClosedDelegate& Delegate)
{
	LogSteamCoreVerbose("");
	SteamFriends()->ActivateGameOverlayToUser(TCHAR_TO_UTF8(TEXT("steamid")), static_cast<const FUniqueNetIdSteam&>(Requestee));

	m_ProfileUIClosedDelegate = Delegate;
	return true;
}

bool FOnlineExternalUISteamCore::ShowAccountUpgradeUI(const FUniqueNetId& UniqueId)
{
	LogSteamCoreVerbose("");
	return false;
}

bool FOnlineExternalUISteamCore::ShowStoreUI(int32 LocalUserNum, const FShowStoreParams& ShowParams, const FOnShowStoreUIClosedDelegate& Delegate)
{
	LogSteamCoreVerbose("");
	if (!ShowParams.ProductId.IsNumeric() || ShowParams.ProductId.IsEmpty())
	{
		return false;
	}

	const uint32 ProductId = static_cast<uint32>(FCString::Atoi(*ShowParams.ProductId));

	if (ProductId == 0)
	{
		return false;
	}

	SteamFriends()->ActivateGameOverlayToStore(ProductId, ShowParams.bAddToCart ? k_EOverlayToStoreFlag_AddToCartAndShow : k_EOverlayToStoreFlag_None);
	m_ShowStoreClosedDelegate = Delegate;

	return true;
}

bool FOnlineExternalUISteamCore::ShowSendMessageUI(int32 LocalUserNum, const FShowSendMessageParams& ShowParams, const FOnShowSendMessageUIClosedDelegate& Delegate)
{
	LogSteamCoreVerbose("");
	return false;
}

bool FOnlineExternalUISteamCore::ShowSendMessageToUserUI(int32 LocalUserNum, const FUniqueNetId& Recipient, const FShowSendMessageParams& ShowParams, const FOnShowSendMessageUIClosedDelegate& Delegate)
{
	LogSteamCoreVerbose("");
	const FUniqueNetIdSteam& TargetUser = static_cast<const FUniqueNetIdSteam&>(Recipient);
	const FString MessageToSend = ShowParams.DisplayMessage.ToString();

	if (!TargetUser.IsValid() || MessageToSend.IsEmpty())
	{
		return false;
	}
	m_ShowMessageClosedDelegate = Delegate;

	m_bMessageSent = SteamFriends()->ReplyToFriendMessage(TargetUser, TCHAR_TO_UTF8(*MessageToSend));
	SteamFriends()->ActivateGameOverlayToUser(TCHAR_TO_UTF8(TEXT("chat")), TargetUser);

	return true;
}
#endif