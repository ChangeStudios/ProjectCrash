/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemSteamCoreTypes.h"
#if UE_VERSION_NEWER_THAN(5,0,3)
#include "Online/NboSerializer.h"
#else
#include "NboSerializer.h"
#endif

#if WITH_STEAMCORE
#if UE_VERSION_OLDER_THAN(5,1,0)
class FNboSerializeToBufferSteamCore : public FNboSerializeToBuffer
{
public:
	FNboSerializeToBufferSteamCore()
		: FNboSerializeToBuffer(512)
	{
	}

	FNboSerializeToBufferSteamCore(uint32 Size)
		: FNboSerializeToBuffer(Size)
	{
	}

	friend inline FNboSerializeToBufferSteamCore& operator<<(FNboSerializeToBufferSteamCore& Ar, const FOnlineSessionInfoSteamCore& SessionInfo)
	{
		check(SessionInfo.m_HostAddr.IsValid());
		Ar << *SessionInfo.m_HostAddr;
		Ar << SessionInfo.m_SessionId->m_UniqueNetId;
		return Ar;
	}
};

class FNboSerializeFromBufferSteamCore : public FNboSerializeFromBuffer
{
public:
	FNboSerializeFromBufferSteamCore(uint8* Packet, int32 Length)
		: FNboSerializeFromBuffer(Packet, Length)
	{
	}

	friend inline FNboSerializeFromBufferSteamCore& operator>>(FNboSerializeFromBufferSteamCore& Ar, FOnlineSessionInfoSteamCore& SessionInfo)
	{
		check(SessionInfo.m_HostAddr.IsValid());

		Ar >> *SessionInfo.m_HostAddr;

		uint64 SessionId;
		Ar >> SessionId;
		SessionInfo.m_SessionId = FUniqueNetIdSteam::Create(SessionId);

		return Ar;
	}
};
#else
#include "NboSerializerOSS.h"
class FNboSerializeToBufferSteamCore : public FNboSerializeToBufferOSS
{
public:
	FNboSerializeToBufferSteamCore()
		: FNboSerializeToBufferOSS(512)
	{
	}

	FNboSerializeToBufferSteamCore(uint32 Size)
		: FNboSerializeToBufferOSS(Size)
	{
	}

	friend inline FNboSerializeToBufferSteamCore& operator<<(FNboSerializeToBufferSteamCore& Ar, const FOnlineSessionInfoSteamCore& SessionInfo)
	{
		check(SessionInfo.m_HostAddr.IsValid());
		((FNboSerializeToBuffer&)Ar) << *SessionInfo.m_HostAddr;
		((FNboSerializeToBuffer&)Ar) << SessionInfo.m_SessionId->m_UniqueNetId;
		return Ar;
	}
};

class FNboSerializeFromBufferSteamCore : public FNboSerializeFromBufferOSS
{
public:
	FNboSerializeFromBufferSteamCore(uint8* Packet,int32 Length)
		: FNboSerializeFromBufferOSS(Packet,Length)
	{
	}

	friend inline FNboSerializeFromBufferSteamCore& operator>>(FNboSerializeFromBufferSteamCore& Ar, FOnlineSessionInfoSteamCore& SessionInfo)
	{
		check(SessionInfo.m_HostAddr.IsValid());

		Ar >> *SessionInfo.m_HostAddr;

		uint64 SessionId;
		Ar >> SessionId;
		SessionInfo.m_SessionId = FUniqueNetIdSteam::Create(SessionId);

		return Ar;
	}
};
#endif
#endif