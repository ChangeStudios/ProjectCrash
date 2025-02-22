/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#pragma once

#include "CoreMinimal.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSteamAuthFailure, const FUniqueNetId&);
typedef FOnSteamAuthFailure::FDelegate FOnSteamAuthFailureDelegate;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSteamAuthResult, const FUniqueNetId&, bool);
typedef FOnSteamAuthResult::FDelegate FOnSteamAuthResultDelegate;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnSteamAuthResultWithCode, const FUniqueNetId&, bool, ESteamAuthResponseCode);
typedef FOnSteamAuthResultWithCode::FDelegate FOnSteamAuthResultWithCodeDelegate;

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAuthUtilsSteamCore
{
public:
	~FOnlineAuthUtilsSteamCore()
	{
		m_OverrideFailureDelegate.Unbind();
		m_OnAuthenticationResultDelegate.Unbind();
		m_OnAuthenticationResultWithCodeDelegate.Unbind();
	}

	static bool IsSteamAuthEnabled();
public:
	FOnSteamAuthFailureDelegate m_OverrideFailureDelegate;
	FOnSteamAuthResultDelegate m_OnAuthenticationResultDelegate;
	FOnSteamAuthResultWithCodeDelegate m_OnAuthenticationResultWithCodeDelegate;
};
