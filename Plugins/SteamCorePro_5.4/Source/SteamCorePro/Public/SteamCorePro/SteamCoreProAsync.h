/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official SteamCorePro Documentation: https://eeldev.com
*/

#pragma once

#include "CoreMinimal.h"
#include "OnlineAsyncTaskManager.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "SteamCorePro/Steam.h"
#include "SteamCoreProAsync.generated.h"

class USteamCoreProAsyncAction;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFailure);

#if WITH_STEAMCORE
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCorePro : public FOnlineAsyncTaskBasic<class USteamCoreProSubsystem>
{
public:
	FOnlineAsyncTaskSteamCorePro(const SteamAPICall_t Handle, float Timeout = 10.f)
		: FOnlineAsyncTaskBasic(nullptr)
		, bInit(false)
		, bTimedOut(false)
		, m_CallbackHandle(Handle)
		, m_AsyncObject(nullptr)
		, m_AsyncTimeout(Timeout)
	{
		
	}
	FOnlineAsyncTaskSteamCorePro(const SteamAPICall_t Handle, USteamCoreProAsyncAction* AsyncObject = nullptr, float Timeout = 10.f)
		: FOnlineAsyncTaskBasic(nullptr)
		, bInit(false)
		, bTimedOut(false)
		, m_CallbackHandle(Handle)
		, m_AsyncObject(AsyncObject)
		, m_AsyncTimeout(Timeout)
	{
	}

	virtual ~FOnlineAsyncTaskSteamCorePro() override
	{
	}

private:
	FOnlineAsyncTaskSteamCorePro();
public:
	bool bInit;
	bool bTimedOut;
	SteamAPICall_t m_CallbackHandle;
	USteamCoreProAsyncAction* m_AsyncObject;
protected:
	virtual void Tick() override;
	virtual FString ToString() const override { return "SteamCoreProAyncTask"; }
protected:
	float m_AsyncTimeout = 10.f;
};
#endif

UCLASS()
class STEAMCOREPRO_API USteamCoreProAsyncAction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	virtual void Activate() override;
	virtual void SetReadyToDestroy() override;
};
