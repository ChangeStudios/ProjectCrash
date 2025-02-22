/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official SteamCorePro Documentation: https://eeldev.com
*/

#include "SteamCoreProSettings.h"
#include "SteamCoreProPluginPrivatePCH.h"
#include "SteamCorePro/SteamUtilities.h"
#include "SteamCoreProLogging.h"
#include "SteamCoreSharedStatics.h"

FString DefaultEngineIni = FPaths::ProjectConfigDir() / "DefaultEngine.ini";

USteamCoreProSettings::USteamCoreProSettings()
{
	if (!GConfig->GetBool(TEXT("OnlineSubsystemSteamCore"), TEXT("bEnabled"), bEnabled, GEngineIni))
	{
		bEnabled = true;
	}

	if (!GConfig->GetBool(TEXT("OnlineSubsystemSteamCore"), TEXT("bRelaunchInSteam"), bRelaunchInSteam, GEngineIni))
	{
		bRelaunchInSteam = true;
	}

	if (!GConfig->GetBool(TEXT("OnlineSubsystemSteamCore"), TEXT("bVACEnabled"), bVACEnabled, GEngineIni))
	{
		bVACEnabled = true;
	}

	if (!GConfig->GetBool(TEXT("OnlineSubsystemSteamCore"), TEXT("bAllowP2PPacketRelay"), bAllowP2PPacketRelay, GEngineIni))
	{
		bAllowP2PPacketRelay = true;
	}

	if (!GConfig->GetBool(TEXT("OnlineSubsystemSteamCore"), TEXT("bCopySteamDlls"), bCopySteamDlls, GEngineIni))
	{
		bCopySteamDlls = true;
	}

	if (!GConfig->GetInt(TEXT("OnlineSubsystemSteamCore"), TEXT("P2PConnectionTimeout"), P2PConnectionTimeout, GEngineIni))
	{
		P2PConnectionTimeout = 90;
	}

	if (!GConfig->GetInt(TEXT("OnlineSubsystemSteamCore"), TEXT("SteamAppId"), SteamAppID, GEngineIni))
	{
		SteamAppID = 480;
	}

	if (!GConfig->GetInt(TEXT("OnlineSubsystemSteamCore"), TEXT("SteamDevAppId"), SteamDevAppID, GEngineIni))
	{
		SteamDevAppID = 480;
	}

	if (!GConfig->GetInt(TEXT("URL"), TEXT("Port"), Port, GEngineIni))
	{
		Port = 7777;
	}

	if (!GConfig->GetInt(TEXT("OnlineSubsystemSteamCore"), TEXT("GameServerQueryPort"), GameServerQueryPort, GEngineIni))
	{
		Port = 27015;
	}

	if (!GConfig->GetString(TEXT("OnlineSubsystemSteamCore"), TEXT("GameVersion"), GameVersion, GEngineIni))
	{
		GameVersion = "1.0.0.0";
	}

	if (!GConfig->GetString(TEXT("OnlineSubsystemSteamCore"), TEXT("GameVersion"), GameVersion, GEngineIni))
	{
		GameVersion = "1.0.0.0";
	}

	if (SteamAppID == 0)
	{
		SteamAppID = 480;
	}

	if (SteamDevAppID == 0)
	{
		SteamDevAppID = SteamAppID;
	}

	if (!GConfig->GetBool(TEXT("OnlineSubsystemSteamCore"), TEXT("bOverrideMinHandshakeVersion"), bOverrideMinHandshakeVersion, GEngineIni))
	{
		bOverrideMinHandshakeVersion = true;
	}

	if (!GConfig->GetInt(TEXT("OnlineSubsystemSteamCore"), TEXT("MinHandshakeVersion"), MinHandshakeVersion, GEngineIni))
	{
		MinHandshakeVersion = 0;
	}

	if (!GConfig->GetBool(TEXT("OnlineSubsystemSteamCore"), TEXT("bOverrideCurrentHandshakeVersion"), bOverrideCurrentHandshakeVersion, GEngineIni))
	{
		bOverrideCurrentHandshakeVersion = true;
	}

	if (!GConfig->GetInt(TEXT("OnlineSubsystemSteamCore"), TEXT("CurrentHandshakeVersion"), CurrentHandshakeVersion, GEngineIni))
	{
		CurrentHandshakeVersion = 0;
	}
}

#if WITH_EDITOR
void USteamCoreProSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	SaveToIni();
}

FName USteamCoreProSettings::GetCategoryName() const
{
	return NAME_Game;
}

void USteamCoreProSettings::SaveToIni()
{
	FString EngineIni = SteamCoreSharedStatics::GetEngineIniPath();
	
	GConfig->SetBool(TEXT("OnlineSubsystemSteamCore"), TEXT("bEnabled"), bEnabled, EngineIni);
	GConfig->SetBool(TEXT("OnlineSubsystemSteamCore"), TEXT("bRelaunchInSteam"), bRelaunchInSteam, EngineIni);
	GConfig->SetBool(TEXT("OnlineSubsystemSteamCore"), TEXT("bVACEnabled"), bVACEnabled, EngineIni);
	GConfig->SetBool(TEXT("OnlineSubsystemSteamCore"), TEXT("bAllowP2PPacketRelay"), bAllowP2PPacketRelay, EngineIni);
	GConfig->SetBool(TEXT("OnlineSubsystemSteamCore"), TEXT("bCopySteamDlls"), bCopySteamDlls, EngineIni);
	GConfig->SetInt(TEXT("OnlineSubsystemSteamCore"), TEXT("P2PConnectionTimeout"), P2PConnectionTimeout, EngineIni);
	GConfig->SetInt(TEXT("OnlineSubsystemSteamCore"), TEXT("SteamAppId"), SteamAppID, EngineIni);
	GConfig->SetInt(TEXT("OnlineSubsystemSteamCore"), TEXT("SteamDevAppId"), SteamDevAppID, EngineIni);
	GConfig->SetInt(TEXT("URL"), TEXT("Port"), Port, EngineIni);
	GConfig->SetInt(TEXT("OnlineSubsystemSteamCore"), TEXT("GameServerQueryPort"), GameServerQueryPort, EngineIni);
	GConfig->SetString(TEXT("OnlineSubsystemSteamCore"), TEXT("GameVersion"), *GameVersion, EngineIni);
	GConfig->SetBool(TEXT("OnlineSubsystemSteamCore"), TEXT("bOverrideMinHandshakeVersion"), bOverrideMinHandshakeVersion, EngineIni);
	GConfig->SetInt(TEXT("OnlineSubsystemSteamCore"), TEXT("MinHandshakeVersion"), MinHandshakeVersion, EngineIni);
	GConfig->SetBool(TEXT("OnlineSubsystemSteamCore"), TEXT("bOverrideCurrentHandshakeVersion"), bOverrideCurrentHandshakeVersion, EngineIni);
	GConfig->SetInt(TEXT("OnlineSubsystemSteamCore"), TEXT("CurrentHandshakeVersion"), CurrentHandshakeVersion, EngineIni);

	GConfig->Flush(false, EngineIni);
	SaveConfig(CPF_Config, *EngineIni);

	UE_LOG(LogSteamCorePro, Log, TEXT("Steam Settings Saved!"));
}
#endif
