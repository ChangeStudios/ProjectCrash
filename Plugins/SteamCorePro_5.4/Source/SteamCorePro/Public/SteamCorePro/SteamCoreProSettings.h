/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official SteamCorePro Documentation: https://eeldev.com
*/

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "SteamCoreProSettings.generated.h"

UCLASS(meta = (DisplayName = "SteamCorePro Plugin"))
class STEAMCOREPRO_API USteamCoreProSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	USteamCoreProSettings();
public:
	/**
	* Enable steam
	* 
	* Make sure you add this to your DefaultEngine.ini:
	* [OnlineSubsystem]
	* DefaultPlatformService=Steam
	*/
	UPROPERTY(EditAnywhere, Category = "Steam Settings")
	bool bEnabled;

	/**
	* Should the game force a relaunch in Steam if the client isn't already loaded
	*/
	UPROPERTY(EditAnywhere, Category = "Steam Settings")
	bool bRelaunchInSteam;

	/**
	* Enable Steam VAC
	*/
	UPROPERTY(EditAnywhere, Category = "Steam Settings")
	bool bVACEnabled;

	/**
	* Should Steam P2P sockets all fall back to Steam servers relay if a direct connection fails
	* Only usable with the SteamCoreSocketsNetDriver
	*
	* Listen servers always use this setting regardless of what this is set to
	*/
	UPROPERTY(EditAnywhere, Category = "Steam Settings", meta = (DisplayName = "Use P2P Packet Relay on Dedicated Servers"))
	bool bAllowP2PPacketRelay;

	/**
	* Copy steamclient dlls when packaging a dedicated server configuration
	* Only windows is supported at this time.
	*
	* Steam Client must be installed on the machine that is packaging.
	* This will throw packaging errors if enabled and steam dlls are not found.
	*/
	UPROPERTY(EditAnywhere, Category = "Steam Settings", meta = (DisplayName = "Copy Steam Dlls (Dedicated Server, Windows Server Only)"))
	bool bCopySteamDlls;

	/**
	* Timeout (in seconds) period for any P2P session
	*/
	UPROPERTY(EditAnywhere, Category = "Steam Settings")
	int P2PConnectionTimeout;

	/**
	* AppID of your application
	*/
	UPROPERTY(EditAnywhere, Category = "Steam Settings")
	int SteamAppID;

	/**
	* SteamDevAppID of your application
	*/
	UPROPERTY(EditAnywhere, Category = "Steam Settings")
	int SteamDevAppID;

	/**
	* Steam Server Port
	*/
	UPROPERTY(EditAnywhere, Category = "Steam Settings")
	int Port;

	/**
	* Steam Server Query Port
	*/
	UPROPERTY(EditAnywhere, Category = "Steam Settings")
	int GameServerQueryPort;

	/**
	* Current game version
	*/
	UPROPERTY(EditAnywhere, Category = "Steam Settings")
	FString GameVersion;

	/**
	*/
	UPROPERTY(EditAnywhere, Category = "Steam Settings", meta = (DisplayName = "Override net.MinHandshakeVersion"))
	bool bOverrideMinHandshakeVersion;
	
	/**
	 * Override console variable: net.MinHandshakeVersion
	*/
	UPROPERTY(EditAnywhere, Category = "Steam Settings", meta = (DisplayName = "net.MinHandshakeVersion", EditCondition = "bOverrideMinHandshakeVersion", EditConditionHides))
	int MinHandshakeVersion;

	/**
	*/
	UPROPERTY(EditAnywhere, Category = "Steam Settings", meta = (DisplayName = "Override net.CurrentHandshakeVersion"))
	bool bOverrideCurrentHandshakeVersion;
	
	/**
	 * Override console variable: net.CurrentHandshakeVersion
	*/
	UPROPERTY(EditAnywhere, Category = "Steam Settings", meta = (DisplayName = "net.CurrentHandshakeVersion", EditCondition = "bOverrideCurrentHandshakeVersion", EditConditionHides))
	int CurrentHandshakeVersion;

private:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual FName GetCategoryName() const override;
	void SaveToIni();
#endif
};