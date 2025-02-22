/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#pragma once

#include "Delegates/Delegate.h"
#include "Delegates/DelegateCombinations.h"
#include "OnlineSubsystemSteamCore.h"

class FOnlineSubsystemSteamCore;
enum class ESteamEncryptedAppTicketState;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnEncryptedAppTicketResponse, bool, int32);

#if WITH_STEAMCORE
class ONLINESUBSYSTEMSTEAMCORE_API FOnlineEncryptedAppTicketSteamCore : public FSelfRegisteringExec
{
PACKAGE_SCOPE:
	FOnlineEncryptedAppTicketSteamCore(FOnlineSubsystemSteamCore* InSubsystem);
	virtual ~FOnlineEncryptedAppTicketSteamCore() override;

	void OnAPICallComplete(bool bEncryptedDataAvailable, int32 ResultCode);

public:
	bool RequestEncryptedAppTicket(void* DataToEncrypt, int SizeOfDataToEncrypt);
	bool GetEncryptedAppTicket(TArray<uint8>& OutEncryptedData) const;

	// FSelfRegisteringExec
	virtual bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) override;

public:
	FOnEncryptedAppTicketResponse m_OnEncryptedAppTicketResultDelegate;
private:
	FOnlineSubsystemSteamCore* m_SteamSubsystem;
	ESteamEncryptedAppTicketState m_TicketState;
};
#endif
