/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#pragma once

#if WITH_STEAMCORE
class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncEventSteamCoreExternalUITriggered : public FOnlineAsyncEvent<FOnlineSubsystemSteamCore>
{
private:
	FOnlineAsyncEventSteamCoreExternalUITriggered()
		: FOnlineAsyncEvent(nullptr),
		  m_bIsActive(false)
	{
	}

	bool m_bIsActive;
public:
	FOnlineAsyncEventSteamCoreExternalUITriggered(FOnlineSubsystemSteamCore* InSteamSubsystem, bool bInIsActive)
		: FOnlineAsyncEvent(InSteamSubsystem),
		  m_bIsActive(bInIsActive)
	{
	}

	virtual ~FOnlineAsyncEventSteamCoreExternalUITriggered() override
	{
	}

	virtual FString ToString() const override;
	virtual void TriggerDelegates() override;
};
#endif
