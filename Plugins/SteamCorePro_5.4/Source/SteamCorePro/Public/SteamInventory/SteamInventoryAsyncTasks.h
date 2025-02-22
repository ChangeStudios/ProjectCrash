/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official Steamworks Documentation: https://partner.steamgames.com/doc/api/ISteamInventory
*/

#pragma once

#include "SteamCorePro/SteamCoreProAsync.h"
#include "SteamInventoryTypes.h"

#if WITH_STEAMCORE
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProInventoryRequestEligiblePromoItemDefinitionsIDs
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProInventoryRequestEligiblePromoItemDefinitionsIDs : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnRequestEligiblePromoItemDefinitionsIDs m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProInventoryRequestEligiblePromoItemDefinitionsIDs(const FOnRequestEligiblePromoItemDefinitionsIDs Callback, const FSteamID SteamId, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , m_SteamID(SteamId)
	{
	}

	FOnlineAsyncTaskSteamCoreProInventoryRequestEligiblePromoItemDefinitionsIDs(USteamCoreProAsyncAction* AsyncObject, const FSteamID SteamId, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_CallbackResults()
		  , m_SteamID(SteamId)
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
	}

private:
	FOnlineAsyncTaskSteamCoreProInventoryRequestEligiblePromoItemDefinitionsIDs() = delete;
protected:
	SteamInventoryEligiblePromoItemDefIDs_t m_CallbackResults;
	FSteamID m_SteamID;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProInventoryRequestEligiblePromoItemDefinitionsIDs")); }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProInventoryRequestPricesResult
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProInventoryRequestPricesResult : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnSteamInventoryRequestPricesResult m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProInventoryRequestPricesResult(const FOnSteamInventoryRequestPricesResult Callback, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
	{
	}

	FOnlineAsyncTaskSteamCoreProInventoryRequestPricesResult(USteamCoreProAsyncAction* AsyncObject, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_CallbackResults()
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
	}

private:
	FOnlineAsyncTaskSteamCoreProInventoryRequestPricesResult() = delete;
protected:
	SteamInventoryRequestPricesResult_t m_CallbackResults;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProInventoryRequestPricesResult")); }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProInventoryStartPurchaseResult
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProInventoryStartPurchaseResult : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnSteamInventoryStartPurchaseResult m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProInventoryStartPurchaseResult(const FOnSteamInventoryStartPurchaseResult Callback, const TArray<FSteamItemDef> ItemDefs, const TArray<int32> Quantity, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , m_ItemDefs(ItemDefs)
		  , m_Quantity(Quantity)
	{
	}

	FOnlineAsyncTaskSteamCoreProInventoryStartPurchaseResult(USteamCoreProAsyncAction* AsyncObject, const TArray<FSteamItemDef> ItemDefs, const TArray<int32> Quantity, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_CallbackResults()
		  , m_ItemDefs(ItemDefs)
		  , m_Quantity(Quantity)
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
	}

private:
	FOnlineAsyncTaskSteamCoreProInventoryStartPurchaseResult() = delete;
protected:
	SteamInventoryStartPurchaseResult_t m_CallbackResults;
	TArray<FSteamItemDef> m_ItemDefs;
	TArray<int32> m_Quantity;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProInventoryStartPurchaseResult")); }
};
#endif