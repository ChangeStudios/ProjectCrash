/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official Steamworks Documentation: https://partner.steamgames.com/doc/api/ISteamTimeline
*/

#include "SteamTimeline/SteamTimeline.h"
#include "SteamCoreProPluginPrivatePCH.h"

USteamProTimeline::USteamProTimeline()
{
#if WITH_STEAMCORE
	SteamTimelineEventRecordingExistsCallback.Register(this, &USteamProTimeline::OnSteamTimelineEventRecordingExists);

	if (IsRunningDedicatedServer())
	{
		SteamTimelineEventRecordingExistsCallback.SetGameserverFlag();
	}
#endif
}

USteamProTimeline::~USteamProTimeline()
{
#if WITH_STEAMCORE
	SteamTimelineEventRecordingExistsCallback.Unregister();
#endif
}

USteamProTimeline* USteamProTimeline::GetSteamTimeline()
{
#if WITH_STEAMCORE
	return SteamTimeline() ? ThisClass::StaticClass()->GetDefaultObject<USteamProTimeline>() : nullptr;
#endif
	return nullptr;
}

void USteamProTimeline::SetTimelineTooltip(FString Description, float Delta)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (GetTimeline())
	{
		GetTimeline()->SetTimelineTooltip(TCHAR_TO_UTF8(*Description), Delta);
	}
#endif
}

void USteamProTimeline::ClearTimelineTooltip(float Delta)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (GetTimeline())
	{
		GetTimeline()->ClearTimelineTooltip(Delta);
	}
#endif
}

FTimelineEventHandle USteamProTimeline::AddInstantaneousTimelineEvent(FString Title, FString Description, FString Icon, int32 IconPriority, float StartOffsetSeconds, ESteamTimelineEventClipPriority PossibleClip)
{
	LogSteamCoreVerbose("");

	FTimelineEventHandle Result;
	
#if WITH_STEAMCORE
	if (GetTimeline())
	{
		Result = GetTimeline()->AddInstantaneousTimelineEvent(TCHAR_TO_UTF8(*Title), TCHAR_TO_UTF8(*Description), TCHAR_TO_UTF8(*Icon), IconPriority, StartOffsetSeconds, static_cast<ETimelineEventClipPriority>(PossibleClip));
	}
#endif

	return Result;
}

FTimelineEventHandle USteamProTimeline::AddRangeTimelineEvent(FString Title, FString Description, FString Icon, int32 IconPriority, float StartOffsetSeconds, float Duration, ESteamTimelineEventClipPriority PossibleClip)
{
	LogSteamCoreVerbose("");

	FTimelineEventHandle Result;
	
#if WITH_STEAMCORE
	if (GetTimeline())
	{
		Result = GetTimeline()->AddRangeTimelineEvent(TCHAR_TO_UTF8(*Title), TCHAR_TO_UTF8(*Description), TCHAR_TO_UTF8(*Icon), IconPriority, StartOffsetSeconds, Duration, static_cast<ETimelineEventClipPriority>(PossibleClip));
	}
#endif

	return Result;
}

FTimelineEventHandle USteamProTimeline::StartRangeTimelineEvent(FString Title, FString Description, FString Icon, int32 IconPriority, float StartOffsetSeconds, ESteamTimelineEventClipPriority PossibleClip)
{
	LogSteamCoreVerbose("");

	FTimelineEventHandle Result;
	
#if WITH_STEAMCORE
	if (GetTimeline())
	{
		Result = GetTimeline()->StartRangeTimelineEvent(TCHAR_TO_UTF8(*Title), TCHAR_TO_UTF8(*Description), TCHAR_TO_UTF8(*Icon), IconPriority, StartOffsetSeconds, static_cast<ETimelineEventClipPriority>(PossibleClip));
	}
#endif

	return Result;
}

void USteamProTimeline::UpdateRangeTimelineEvent(FTimelineEventHandle Event, FString Title, FString Description, FString Icon, int32 Priority, ESteamTimelineEventClipPriority PossibleClip)
{
	LogSteamCoreVerbose("");
	
#if WITH_STEAMCORE
	if (GetTimeline())
	{
		GetTimeline()->UpdateRangeTimelineEvent(Event, TCHAR_TO_UTF8(*Title), TCHAR_TO_UTF8(*Description), TCHAR_TO_UTF8(*Icon), Priority, static_cast<ETimelineEventClipPriority>(PossibleClip));
	}
#endif
}

void USteamProTimeline::EndRangeTimelineEvent(FTimelineEventHandle Event, float EndOffsetSeconds)
{
	LogSteamCoreVerbose("");
	
#if WITH_STEAMCORE
	if (GetTimeline())
	{
		GetTimeline()->EndRangeTimelineEvent(Event, EndOffsetSeconds);
	}
#endif
}

void USteamProTimeline::RemoveTimelineEvent(FTimelineEventHandle Event)
{
	LogSteamCoreVerbose("");
	
#if WITH_STEAMCORE
	if (GetTimeline())
	{
		GetTimeline()->RemoveTimelineEvent(Event);
	}
#endif
}

void USteamProTimeline::SetTimelineGameMode(ESteamTimelineGameMode Mode)
{
	LogSteamCoreVerbose("");
	
#if WITH_STEAMCORE
	if (GetTimeline())
	{
		GetTimeline()->SetTimelineGameMode(static_cast<ETimelineGameMode>(Mode));
	}
#endif
}

#if WITH_STEAMCORE
void USteamProTimeline::OnSteamTimelineEventRecordingExists(SteamTimelineEventRecordingExists_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		SteamTimelineEventRecordingExists.Broadcast(Data);
	});
}

USteamProTimelineGamePhases::USteamProTimelineGamePhases()
{
#if WITH_STEAMCORE
	SteamTimelineGamePhaseRecordingExistsCallback.Register(this, &USteamProTimelineGamePhases::OnSteamTimelineGamePhaseRecordingExists);

	if (IsRunningDedicatedServer())
	{
		SteamTimelineGamePhaseRecordingExistsCallback.SetGameserverFlag();
	}
#endif
}

USteamProTimelineGamePhases::~USteamProTimelineGamePhases()
{
#if WITH_STEAMCORE
	SteamTimelineGamePhaseRecordingExistsCallback.Unregister();
#endif
}
#endif

USteamProTimelineGamePhases* USteamProTimelineGamePhases::GetSteamTimelineGamePhase()
{
#if WITH_STEAMCORE
	return SteamTimeline() ? ThisClass::StaticClass()->GetDefaultObject<USteamProTimelineGamePhases>() : nullptr;
#endif
	return nullptr;
}

void USteamProTimelineGamePhases::StartGamePhase()
{
	LogSteamCoreVerbose("");
	
#if WITH_STEAMCORE
	if (GetTimeline())
	{
		GetTimeline()->StartGamePhase();
	}
#endif
}

void USteamProTimelineGamePhases::EndGamePhase()
{
	LogSteamCoreVerbose("");
	
#if WITH_STEAMCORE
	if (GetTimeline())
	{
		GetTimeline()->EndGamePhase();
	}
#endif
}

void USteamProTimelineGamePhases::SetGamePhaseID(FString PhaseID)
{
}

void USteamProTimelineGamePhases::AddGamePhaseTag(FString TagName, FString TagIcon, FString TagGroup, int32 Priority)
{
}

void USteamProTimelineGamePhases::SetGamePhaseAttribute(FString AttributeGroup, FString AttributeValue, int32 Priority)
{
}

void USteamProTimelineGamePhases::OpenOverlayToGamePhase(FString PhaseId)
{
}

void USteamProTimelineGamePhases::OpenOverlayToTimelineEvent(FTimelineEventHandle Event)
{
}

#if WITH_STEAMCORE
void USteamProTimelineGamePhases::OnSteamTimelineGamePhaseRecordingExists(SteamTimelineGamePhaseRecordingExists_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		SteamTimelineGamePhaseRecordingExists.Broadcast(Data);
	});
}
#endif