/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#include "Voice/VoiceEngineSteamCore.h"
#include "OnlineSubsystemSteamCorePrivatePCH.h"
#include "SteamCoreSharedAudioSubsystem.h"
#include "VoiceModule.h"

#if WITH_STEAMCORE

FVoiceEngineSteamCore::FVoiceEngineSteamCore(IOnlineSubsystem* InSubsystem)
	: FVoiceEngineImpl(InSubsystem),
	  m_SteamUserPtr(SteamUser()),
	  m_SteamFriendsPtr(SteamFriends())
{
}

FVoiceEngineSteamCore::~FVoiceEngineSteamCore()
{
	if (FVoiceEngineImpl::IsRecording())
	{
		m_SteamFriendsPtr->SetInGameVoiceSpeaking(m_SteamUserPtr->GetSteamID(), false);
	}
}

void FVoiceEngineSteamCore::StartRecording() const
{
	LogSteamCoreVerbose("FVoiceEngineSteamCoreCore::StartRecording");
	if (GetVoiceCapture().IsValid())
	{
		if (!GetVoiceCapture()->Start())
		{
			LogSteamCoreWarn("Failed to start voice recording");
		}
		else if (m_SteamFriendsPtr)
		{
			m_SteamFriendsPtr->SetInGameVoiceSpeaking(m_SteamUserPtr->GetSteamID(), true);
		}
	}
}

void FVoiceEngineSteamCore::StoppedRecording() const
{
	LogSteamCoreVerbose("FVoiceEngineSteamCoreCore::StoppedRecording");
	if (m_SteamFriendsPtr)
	{
		m_SteamFriendsPtr->SetInGameVoiceSpeaking(m_SteamUserPtr->GetSteamID(), false);
	}
}

uint32 FVoiceEngineSteamCore::RegisterLocalTalker(uint32 LocalUserNum)
{
	LogSteamCoreVerbose("FVoiceEngineSteamCoreCore::RegisterLocalTalker");

	if (!GetVoiceCapture().IsValid())
	{
		FString DeviceId;
		FString DeviceName;
		TArray<FSteamCoreAudioInputDeviceInfo> InputDevices = USteamCoreSharedAudioSubsystem::GetAudioInputDevices();
		GConfig->GetString(TEXT("OnlineSubsystemSteamCore"), TEXT("VoiceInput"), DeviceId, GGameIni);

		if (DeviceId.Len() > 0)
		{
			FSteamCoreAudioInputDeviceInfo AudioInputDeviceInfo = USteamCoreSharedAudioSubsystem::FindAudioDeviceInfoFromDeviceId(DeviceId); 
			DeviceName = AudioInputDeviceInfo.DeviceName;
		}

		if (DeviceName.Len() > 0)
		{
			LogSteamCoreVerbose("Creating Voice Capture Engine for device (%s)", *DeviceName);
			
			GetVoiceCapture() = FVoiceModule::Get().CreateVoiceCapture(DeviceName);

			if (!GetVoiceCapture().IsValid())
			{
				LogSteamCoreWarn("RegisterLocalTalker: Failed to create a Voice Capture Device (%s)", *DeviceName);
			}
		}
		else
		{
			LogSteamCoreWarn("RegisterLocalTalker: Trying to create Voice Capture using default device");

			GetVoiceCapture() = FVoiceModule::Get().CreateVoiceCapture("");
		}
		
		if (!GetVoiceCapture().IsValid())
		{
			LogSteamCoreWarn("RegisterLocalTalker: Failed to create a Voice Capture Device");
			return ONLINE_FAIL;
		}
	}

	FVoiceEngineImpl::RegisterLocalTalker(LocalUserNum);

	if (IsOwningUser(LocalUserNum))
	{
		return ONLINE_SUCCESS;
	}
	else
	{
		return ONLINE_FAIL;
	}
}
#endif
