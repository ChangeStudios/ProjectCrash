/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#include "SteamCoreSharedAudioSubsystem.h"
#include "AudioCaptureCore.h"
#include "AudioThread.h"
#include "SteamCoreSharedLogging.h"
#include "SteamCoreSharedModule.h"

TArray<FSteamCoreAudioInputDeviceInfo> USteamCoreSharedAudioSubsystem::s_AvailableDeviceInfos;

FSteamCoreAudioInputDeviceInfo::FSteamCoreAudioInputDeviceInfo(const Audio::FCaptureDeviceInfo& InDeviceInfo) :
	DeviceName(InDeviceInfo.DeviceName),
	DeviceId(InDeviceInfo.DeviceId),
	InputChannels(InDeviceInfo.InputChannels),
	PreferredSampleRate(InDeviceInfo.PreferredSampleRate),
	bSupportsHardwareAEC(InDeviceInfo.bSupportsHardwareAEC)
{
}

FString USteamCoreAudioCaptureBlueprintLibrary::Conv_AudioInputDeviceInfoToString(const FSteamCoreAudioInputDeviceInfo& InDeviceInfo)
{
	FString output = FString::Printf(TEXT("Device Name: %s, \nDevice Id: %s, \nNum Channels: %u, \nSample Rate: %u, \nSupports Hardware AEC: %u, \n"),
		*InDeviceInfo.DeviceName, *InDeviceInfo.DeviceId, InDeviceInfo.InputChannels, InDeviceInfo.PreferredSampleRate, InDeviceInfo.bSupportsHardwareAEC);

	return output;
}


void USteamCoreAudioCaptureBlueprintLibrary::GetAvailableAudioInputDevices(const UObject* WorldContextObject, const FOnSteamCoreAudioInputDevicesObtained& OnObtainDevicesEvent)
{
	if (!IsInAudioThread())
	{
		//Send this over to the audio thread, with the same settings
		FAudioThread::RunCommandOnAudioThread([WorldContextObject, OnObtainDevicesEvent]()
			{
				GetAvailableAudioInputDevices(WorldContextObject, OnObtainDevicesEvent);
			});

		return;
	}

	TArray<FSteamCoreAudioInputDeviceInfo> AvailableDeviceInfos; //The array of audio device info to return

	Audio::FAudioCapture AudioCapture;
	TArray<Audio::FCaptureDeviceInfo> InputDevices;

	AudioCapture.GetCaptureDevicesAvailable(InputDevices);

	for (auto Iter = InputDevices.CreateConstIterator(); Iter; ++Iter)
	{
		AvailableDeviceInfos.Add(FSteamCoreAudioInputDeviceInfo(*Iter));
	}

	//Call delegate event, and send the info there
	OnObtainDevicesEvent.Execute(AvailableDeviceInfos);
}

void USteamCoreSharedAudioSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LogSteamCoreVerbose("USteamCoreSharedAudioSubsystem::Initialize");

	GatherAudioInputDevices();
}

void USteamCoreSharedAudioSubsystem::GatherAudioInputDevices()
{
	LogSteamCoreVerbose("FOnlineVoiceSteamCore::GatherAudioInputDevices");

	s_AvailableDeviceInfos.Empty();

	if (!IsInAudioThread())
	{
		FAudioThread::RunCommandOnAudioThread([this]()
		{
			GatherAudioInputDevices();
		});

		return;
	}

	Audio::FAudioCapture AudioCapture;
	TArray<Audio::FCaptureDeviceInfo> InputDevices;

	AudioCapture.GetCaptureDevicesAvailable(InputDevices);

	for (auto Iter = InputDevices.CreateConstIterator(); Iter; ++Iter)
	{
		FSteamCoreAudioInputDeviceInfo DeviceInfo = FSteamCoreAudioInputDeviceInfo(*Iter);
		s_AvailableDeviceInfos.Add(DeviceInfo);
		LogSteamCoreVerbose("Found Input Device: %s", *USteamCoreAudioCaptureBlueprintLibrary::Conv_AudioInputDeviceInfoToString(DeviceInfo));
	}

	LogSteamCoreVerbose("Found %i Input Devices", s_AvailableDeviceInfos.Num());
}

FSteamCoreAudioInputDeviceInfo USteamCoreSharedAudioSubsystem::FindAudioDeviceInfoFromDeviceId(FString DeviceId)
{
	LogSteamCoreVerbose("FindAudioDeviceInfoFromDeviceId DeviceId: (%s)", *DeviceId);
	
	FSteamCoreAudioInputDeviceInfo Result;
	
	LogSteamCoreVerbose("Iterating Input Devices..");
	for (auto Element : s_AvailableDeviceInfos)
	{
		LogSteamCoreVerbose("Input Device: %s", *USteamCoreAudioCaptureBlueprintLibrary::Conv_AudioInputDeviceInfoToString(Result));
		
		if (DeviceId == Element.DeviceId)
		{
			Result = Element;
			break;
		}
	}
	
	LogSteamCoreVerbose("FindAudioDeviceInfoFromDeviceId: %s", *USteamCoreAudioCaptureBlueprintLibrary::Conv_AudioInputDeviceInfoToString(Result));
	
	return Result;
}

FSteamCoreAudioInputDeviceInfo USteamCoreSharedAudioSubsystem::K2_FindAudioDeviceInfoFromDeviceId(FString DeviceId)
{
	return FindAudioDeviceInfoFromDeviceId(DeviceId);
}

TArray<FSteamCoreAudioInputDeviceInfo> USteamCoreSharedAudioSubsystem::K2_GetAudioInputDevices()
{
	return GetAudioInputDevices();
}

FSteamCoreAudioInputDeviceInfo USteamCoreSharedAudioSubsystem::K2_FindAudioDeviceInfoFromDeviceName(FString DeviceName)
{
	FSteamCoreAudioInputDeviceInfo Result;
	
	for (auto Element : s_AvailableDeviceInfos)
	{
		if (DeviceName == Element.DeviceName)
		{
			Result = Element;
			break;
		}
	}
	return Result;
}
