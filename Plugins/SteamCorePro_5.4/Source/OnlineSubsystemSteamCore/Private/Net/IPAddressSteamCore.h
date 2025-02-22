/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#pragma once

#include "CoreMinimal.h"
#include "IPAddress.h"
#include "OnlineSubsystemSteamCore.h"
#include "OnlineSubsystemSteamCoreTypes.h"
#include "OnlineSubsystemSteamCorePackage.h"

#if WITH_STEAMCORE
class FInternetAddrSteamCore : public FInternetAddr
{
PACKAGE_SCOPE:
	FUniqueNetIdSteamRef m_SteamId;
	int32 m_SteamChannel;

	FInternetAddrSteamCore(const FInternetAddrSteamCore& Src)
		: m_SteamId(Src.m_SteamId),
		  m_SteamChannel(Src.m_SteamChannel)
	{
	}

public:
	FInternetAddrSteamCore()
		: m_SteamId(FUniqueNetIdSteam::EmptyId()),
		  m_SteamChannel(0)
	{
	}

	explicit FInternetAddrSteamCore(const FUniqueNetIdSteam& InSteamId)
		: m_SteamId(InSteamId.AsShared()),
		  m_SteamChannel(0)
	{
	}

	explicit FInternetAddrSteamCore(const FUniqueNetIdSteamRef& InSteamId)
		: m_SteamId(InSteamId),
		  m_SteamChannel(0)
	{
	}

	virtual TArray<uint8> GetRawIp() const override;
	virtual void SetRawIp(const TArray<uint8>& RawAddr) override;

	virtual void SetIp(uint32 InAddr) override
	{
		LogSteamCoreWarn("FInternetAddrSteamCore::SetIp is not supported.");
	}

	virtual void SetIp(const TCHAR* InAddr, bool& bIsValid) override;

	virtual void GetIp(uint32& OutAddr) const override
	{
		OutAddr = 0;
		LogSteamCoreWarn("FInternetAddrSteamCore::GetIp is not supported and will set OutAddr to 0.");
	}

	virtual void SetPort(int32 InPort) override
	{
		m_SteamChannel = InPort;
	}

	virtual void GetPort(int32& OutPort) const override
	{
		OutPort = m_SteamChannel;
	}

	virtual int32 GetPort() const override
	{
		return m_SteamChannel;
	}

	virtual void SetAnyAddress() override
	{
	}

	virtual void SetBroadcastAddress() override
	{
	}

	virtual void SetLoopbackAddress() override
	{
	}

	virtual FString ToString(bool bAppendPort) const override
	{
		if (bAppendPort)
		{
			return FString::Printf(TEXT("%lld:%d"), m_SteamId->m_UniqueNetId, m_SteamChannel);
		}
		else
		{
			return FString::Printf(TEXT("%lld"), m_SteamId->m_UniqueNetId);
		}
	}

	virtual bool operator==(const FInternetAddr& Other) const override
	{
		FInternetAddrSteamCore& SteamOther = (FInternetAddrSteamCore&)Other;
		return *m_SteamId == *SteamOther.m_SteamId && m_SteamChannel == SteamOther.m_SteamChannel;
	}

	bool operator==(const FInternetAddrSteamCore& Other) const
	{
		return *m_SteamId == *Other.m_SteamId && m_SteamChannel == Other.m_SteamChannel;
	}

	bool operator!=(const FInternetAddrSteamCore& Other) const
	{
		return !(FInternetAddrSteamCore::operator==(Other));
	}

	virtual uint32 GetTypeHash() const override
	{
#if UE_VERSION_OLDER_THAN(5,2,0)
		return ::GetTypeHash(ToString(true));
#else
		return GetTypeHashHelper(ToString(true));
#endif
	}

	friend uint32 GetTypeHash(const FInternetAddrSteamCore& A)
	{
		return A.GetTypeHash();
	}

	virtual bool IsValid() const override
	{
		return m_SteamId->IsValid();
	}

	virtual FName GetProtocolType() const override;

	virtual TSharedRef<FInternetAddr> Clone() const override
	{
		TSharedRef<FInternetAddrSteamCore> NewAddress = MakeShareable(new FInternetAddrSteamCore);
		NewAddress->m_SteamId = m_SteamId;
		NewAddress->m_SteamChannel = m_SteamChannel;
		return NewAddress;
	}
};
#endif