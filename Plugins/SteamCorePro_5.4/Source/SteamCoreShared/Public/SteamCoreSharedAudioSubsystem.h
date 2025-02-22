/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#pragma once
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SteamCoreSharedTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "SteamCoreSharedAudioSubsystem.generated.h"

class ISoundHandleOwner;

UCLASS(meta = (ScriptName = "SteamCoreAudioCaptureLibrary"))
class STEAMCORESHARED_API USteamCoreAudioCaptureBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Returns the device info in a human readable format
	 * @param info - The audio device data to print
	 * @return The data in a string format
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Audio Input Device Info To String", CompactNodeTitle = "To String", BlueprintAutocast), Category = "Audio")
	static FString Conv_AudioInputDeviceInfoToString(const FSteamCoreAudioInputDeviceInfo& info);

	/**
	 * Gets information about all audio output devices available in the system
	 * @param OnObtainDevicesEvent - the event to fire when the audio endpoint devices have been retrieved
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio", meta = (WorldContext = "WorldContextObject"))
	static void GetAvailableAudioInputDevices(const UObject* WorldContextObject, const FOnSteamCoreAudioInputDevicesObtained& OnObtainDevicesEvent);
};

/**
 * An implementation of ISoundHandleSystem using AudioEngineSubsystem
 */
UCLASS()
class STEAMCORESHARED_API USteamCoreSharedAudioSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	void GatherAudioInputDevices();

	static TArray<FSteamCoreAudioInputDeviceInfo> GetAudioInputDevices() { return s_AvailableDeviceInfos; }
	static FSteamCoreAudioInputDeviceInfo FindAudioDeviceInfoFromDeviceId(FString DeviceId);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Find Audio Device Info From Device Id"), Category = "SteamCore|Voice")
	static FSteamCoreAudioInputDeviceInfo K2_FindAudioDeviceInfoFromDeviceId(FString DeviceId);
	
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Audio Input Devices"), Category = "SteamCore|Voice")
	static TArray<FSteamCoreAudioInputDeviceInfo> K2_GetAudioInputDevices();
	
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Find Audio Device Info From Device Name"), Category = "SteamCore|Voice")
	static FSteamCoreAudioInputDeviceInfo K2_FindAudioDeviceInfoFromDeviceName(FString DeviceName);

private:
	static TArray<FSteamCoreAudioInputDeviceInfo> s_AvailableDeviceInfos;
};
