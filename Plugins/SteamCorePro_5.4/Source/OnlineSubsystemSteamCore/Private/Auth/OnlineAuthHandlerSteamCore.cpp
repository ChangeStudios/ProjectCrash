/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#include "Auth/OnlineAuthHandlerSteamCore.h"
#include "Auth/OnlineAuthHandlerTypesSteamCore.h"
#include "OnlineSubsystemSteamCorePrivatePCH.h"

#if UE_VERSION_NEWER_THAN(5,2,1)
using namespace UE;
#endif

#if !UE_BUILD_SHIPPING
FString ReusableKey;
#endif

#if WITH_STEAMCORE
FSteamCoreAuthHandlerComponent::FSteamCoreAuthHandlerComponent()
	: m_AuthInterface(nullptr),
	  m_SteamUserPtr(SteamUser()),
	  m_State(ESteamAuthHandlerState::Uninitialized),
	  m_bIsEnabled(true),
	  m_LastTimestamp(0.0f),
	  m_TicketHandle(k_HAuthTicketInvalid),
	  m_SteamId(m_SteamUserPtr ? FUniqueNetIdSteam::Create(m_SteamUserPtr->GetSteamID()) : FUniqueNetIdSteam::EmptyId())
{
	SetActive(true);
	bRequiresHandshake = true;

	const FOnlineSubsystemSteamCore* OSS = static_cast<FOnlineSubsystemSteamCore*>(IOnlineSubsystem::Get(STEAMCORE_SUBSYSTEM));
	if (OSS != nullptr)
	{
		m_AuthInterface = OSS->GetAuthInterface();
		if (!m_AuthInterface.IsValid() || !m_AuthInterface->IsSessionAuthEnabled())
		{
			m_bIsEnabled = false;
		}
	}
	else
	{
		m_bIsEnabled = false;
	}
}

FSteamCoreAuthHandlerComponent::~FSteamCoreAuthHandlerComponent()
{
	LogSteamCoreVerbose("");
	if (!m_bIsEnabled || !m_AuthInterface.IsValid())
	{
		return;
	}

	if (Handler->Mode == Handler::Mode::Client)
	{
		m_AuthInterface->RevokeTicket(m_TicketHandle);
	}
	else
	{
		m_AuthInterface->RemoveUser(*m_SteamId);
	}
}

void FSteamCoreAuthHandlerComponent::CountBytes(FArchive& Ar) const
{
	HandlerComponent::CountBytes(Ar);

	const SIZE_T SizeOfThis = sizeof(*this) - sizeof(HandlerComponent);
	Ar.CountBytes(SizeOfThis, SizeOfThis);

	m_UserTicket.CountBytes(Ar);
}

void FSteamCoreAuthHandlerComponent::Initialize()
{
	LogSteamCoreVerbose("");
	if (!m_AuthInterface.IsValid() || !m_AuthInterface->IsSessionAuthEnabled())
	{
		LogSteamCoreWarn("AUTH HANDLER: Deactivating due to missing requirements");
		m_bIsEnabled = false;
		if (Handler != nullptr)
		{
			SetComponentReady();
		}
		else
		{
			SetActive(false);
		}
	}
}

void FSteamCoreAuthHandlerComponent::NotifyHandshakeBegin()
{
	LogSteamCoreVerbose("");
	if (!m_bIsEnabled)
	{
		return;
	}

	if (Handler->Mode == Handler::Mode::Client)
	{
		SetState(ESteamAuthHandlerState::WaitingForServerSteamId);
	}
	else
	{
		SendServerSteamId();
		m_LastTimestamp = FPlatformTime::Seconds();
	}
}

void FSteamCoreAuthHandlerComponent::SendAuthKey(bool bGenerateNewKey)
{
	LogSteamCoreVerbose("");
	FBitWriter AuthDataPacket((sizeof(FSteamAuthUserData) + FOnlineAuthSteamCore::GetMaxTicketSizeInBytes()) * 8 + 1);
	FSteamAuthUserData UserData;
	UserData.m_SteamId = m_SteamId;

	if (bGenerateNewKey || m_TicketHandle == k_HAuthTicketInvalid)
	{
		m_UserTicket = m_AuthInterface->GetAuthTicket(m_TicketHandle);
	}

#if !UE_BUILD_SHIPPING
	if (ReusableKey.IsEmpty())
	{
		ReusableKey = m_UserTicket;
	}
	else if (m_AuthInterface->m_bReuseKey)
	{
		m_UserTicket = ReusableKey;
	}

	if (m_AuthInterface->m_bNeverSendKey)
	{
		SetState(ESteamAuthHandlerState::SentAuthKey);
		return;
	}

	if (m_AuthInterface->m_bBadKey)
	{
		m_UserTicket = TEXT("THIS IS A BAD STEAM KEY");
	}

	if (m_AuthInterface->m_bSendBadId)
	{
		UserData.m_SteamId = FUniqueNetIdSteam::EmptyId();
	}
#endif

	UserData.m_AuthKey = m_UserTicket;

	AuthDataPacket.WriteBit(1);
	AuthDataPacket << UserData;
	SendPacket(AuthDataPacket);

	SetState(ESteamAuthHandlerState::SentAuthKey);
	LogSteamCoreVerbose("AUTH HANDLER: Sending auth key");
}

void FSteamCoreAuthHandlerComponent::SendServerSteamId()
{
	uint64 ServerSteamId = 0;
	if (SteamGameServer())
	{
		ServerSteamId = SteamGameServer()->GetSteamID().ConvertToUint64();
	}
	else
	{
		ServerSteamId = m_SteamUserPtr->GetSteamID().ConvertToUint64();
	}

	LogSteamCoreVerbose("FSteamCoreAuthHandlerComponent::SendServerSteamId (%llu)", ServerSteamId);

	FServerSteamIdData ThePacket;
	FBitWriter ResultPacketWriter(sizeof(FServerSteamIdData) * 8 + 1, true);
	ResultPacketWriter.WriteBit(1);

	ThePacket.m_ServerSteamId = ServerSteamId;
	ResultPacketWriter << ThePacket;

	SendPacket(ResultPacketWriter);

	SetState(ESteamAuthHandlerState::WaitingForKey);
}

bool FSteamCoreAuthHandlerComponent::SendAuthResult()
{
	LogSteamCoreVerbose("");
	const bool AuthStatusResult = m_AuthInterface->AuthenticateUser(*m_SteamId);

	FSteamAuthResult AllowedPacket;
	FBitWriter ResultPacketWriter(sizeof(FSteamAuthResult) * 8 + 1, true);
	ResultPacketWriter.WriteBit(1);

	AllowedPacket.bWasSuccess = AuthStatusResult;
	ResultPacketWriter << AllowedPacket;

	SendPacket(ResultPacketWriter);

	LogSteamCoreVerbose("AUTH HANDLER: Sending auth result to user %s with flag success? %d", *m_SteamId->ToString(), AuthStatusResult);

	return AuthStatusResult;
}

void FSteamCoreAuthHandlerComponent::SendPacket(FBitWriter& OutboundPacket)
{
	LogSteamCoreVerbose("");
#if !UE_BUILD_SHIPPING
	if (m_AuthInterface->m_bBadWrite)
	{
		OutboundPacket.SetError();
	}

	if (m_AuthInterface->m_bDropAll)
	{
		return;
	}

	if (m_AuthInterface->m_bRandomDrop && FMath::RandBool() == false)
	{
		LogSteamCoreWarn("AUTH HANDLER: Random packet was dropped!");
		return;
	}
#endif

	FOutPacketTraits Traits;

	Handler->SendHandlerPacket(this, OutboundPacket, Traits);
	m_LastTimestamp = FPlatformTime::Seconds();
}

void FSteamCoreAuthHandlerComponent::RequestResend()
{
	LogSteamCoreVerbose("");
	FBitWriter ResendWriter(sizeof(FSteamAuthInfoData) * 8 + 1);
	FSteamAuthInfoData ResendingPacket;

	ResendWriter.WriteBit(1);

	ResendingPacket.m_Type = (Handler->Mode == Handler::Mode::Server) ? ESteamAuthMsgType::ResendKey : ESteamAuthMsgType::ResendResult;

	ResendWriter << ResendingPacket;
	SendPacket(ResendWriter);
}

bool FSteamCoreAuthHandlerComponent::IsValid() const
{
	return m_bIsEnabled;
}

void FSteamCoreAuthHandlerComponent::Incoming(FBitReader& Packet)
{
	LogSteamCoreVeryVerbose("");
	const bool bForSteamAuth = !!Packet.ReadBit() && !Packet.IsError();
	if (!m_bIsEnabled || !m_AuthInterface.IsValid() || !bForSteamAuth)
	{
		return;
	}

#if !UE_BUILD_SHIPPING
	if (m_AuthInterface->m_bDropAll)
	{
		Packet.SetError();
		return;
	}
#endif

	FBitReaderMark PacketMarker(Packet);
	FSteamAuthInfoData Header;

	Packet << Header;

	if (Packet.IsError())
	{
		LogSteamCoreError("AUTH HANDLER: Incoming steam auth packet could not be properly serialized.");
		return;
	}

	PacketMarker.Pop(Packet);

	if (m_State == ESteamAuthHandlerState::WaitingForServerSteamId)
	{
		FServerSteamIdData SteamIdData;
		Packet << SteamIdData;

		m_ServerSteamId = SteamIdData.m_ServerSteamId;

		m_AuthInterface->SetServerSteamId(m_ServerSteamId);

		LogSteamCoreVerbose("Received ServerSteamId: (%llu)", m_ServerSteamId);

		SendAuthKey(false);
	}
	else if (m_State == ESteamAuthHandlerState::WaitingForKey && Header.m_Type == ESteamAuthMsgType::Auth)
	{
		FSteamAuthUserData AuthData;
		Packet << AuthData;

		if (Packet.IsError())
		{
			LogSteamCoreWarn("AUTH HANDLER: Packet was marked as error after serializing");
			return;
		}

		m_SteamId = AuthData.m_SteamId;
		if (!m_SteamId->IsValid())
		{
			LogSteamCoreError("AUTH HANDLER: Got an invalid steamid");
			m_AuthInterface->ExecuteResultDelegate(*m_SteamId, false, ESteamAuthResponseCode::NotConnectedToSteam);
			Packet.SetError();
			return;
		}

		const FOnlineAuthSteamCore::SharedAuthUserSteamPtr TargetUser = m_AuthInterface->GetOrCreateUser(*m_SteamId);
		if (!TargetUser.IsValid())
		{
			LogSteamCoreError("AUTH HANDLER: Could not create user listing for %s", *m_SteamId->ToString());
			m_AuthInterface->ExecuteResultDelegate(*m_SteamId, false, ESteamAuthResponseCode::FailedToCreateUser);
			Packet.SetError();
			return;
		}

		TargetUser->SetKey(AuthData.m_AuthKey);

		if (!SendAuthResult())
		{
			m_AuthInterface->MarkPlayerForKick(*m_SteamId);
		}

		SetComponentReady();
	}
	else if (m_State == ESteamAuthHandlerState::SentAuthKey)
	{
		if (Header.m_Type == ESteamAuthMsgType::Result)
		{
			FSteamAuthResult AuthResult;
			Packet << AuthResult;

			LogSteamCoreVerbose("AUTH HANDLER: Got result from server, was success? %d", AuthResult.bWasSuccess);
			SetComponentReady();
		}
		else if (Header.m_Type == ESteamAuthMsgType::ResendKey)
		{
			LogSteamCoreVerbose("AUTH HANDLER: Server requested us to resend our key.");
			SendAuthKey(false);
		}
	}
	else if (Handler && Handler->Mode == Handler::Mode::Server && Header.m_Type == ESteamAuthMsgType::ResendResult)
	{
		if (m_State == ESteamAuthHandlerState::Initialized)
		{
			LogSteamCoreVerbose("AUTH HANDLER: Got request from %s to resend result", *m_SteamId->ToString());
			SendAuthResult();
		}
		else
		{
			LogSteamCoreWarn("AUTH HANDLER: User has not sent ticket and requesting results.");
			RequestResend();
		}
	}
}

void FSteamCoreAuthHandlerComponent::Outgoing(FBitWriter& Packet, FOutPacketTraits& Traits)
{
	LogSteamCoreVeryVerbose("");
#if !UE_BUILD_SHIPPING
	if (m_AuthInterface.IsValid() && m_AuthInterface->m_bDropAll)
	{
		Packet.SetError();
		return;
	}
#endif

	FBitWriter NewPacket(Packet.GetNumBits() + 1, true);

	NewPacket.WriteBit(0);
	NewPacket.SerializeBits(Packet.GetData(), Packet.GetNumBits());

	Packet = MoveTemp(NewPacket);
}

void FSteamCoreAuthHandlerComponent::Tick(float DeltaTime)
{
	//LogSteamCoreVerbose("");
	if (!m_bIsEnabled || m_State == ESteamAuthHandlerState::Initialized || !Handler)
	{
		return;
	}

	const float CurTime = FPlatformTime::Seconds();
	if (m_LastTimestamp != 0.0 && CurTime - m_LastTimestamp > 2.0f)
	{
		RequestResend();
	}
}

int32 FSteamCoreAuthHandlerComponent::GetReservedPacketBits() const
{
	return 1;
}

void FSteamCoreAuthHandlerComponent::SetState(ESteamAuthHandlerState NewState)
{
	switch (NewState) {
	case ESteamAuthHandlerState::Uninitialized:
		LogSteamCoreVerbose("SetState: Uninitialized");
		break;
	case ESteamAuthHandlerState::WaitingForServerSteamId:
		LogSteamCoreVerbose("SetState: WaitingForServerSteamId");
		break;
	case ESteamAuthHandlerState::WaitingForKey:
		LogSteamCoreVerbose("SetState: WaitingForKey");
		break;
	case ESteamAuthHandlerState::SentAuthKey:
		LogSteamCoreVerbose("SetState: SentAuthKey");
		break;
	case ESteamAuthHandlerState::Initialized:
		LogSteamCoreVerbose("SetState: Initialized");
		break;
	}
	
	m_State = NewState;
}

void FSteamCoreAuthHandlerComponent::SetComponentReady()
{
	LogSteamCoreVerbose("");
	if (m_State != ESteamAuthHandlerState::Initialized)
	{
		SetState(ESteamAuthHandlerState::Initialized);
		Initialized();
	}
}
#endif

USteamCoreAuthComponentModuleInterface::USteamCoreAuthComponentModuleInterface(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	LogSteamCoreVerbose("");
}

TSharedPtr<HandlerComponent> USteamCoreAuthComponentModuleInterface::CreateComponentInstance(FString& Options)
{
	LogSteamCoreVerbose("");
#if WITH_STEAMCORE
	return MakeShareable(new FSteamCoreAuthHandlerComponent);
#endif
	return nullptr;
}
