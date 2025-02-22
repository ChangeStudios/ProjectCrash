/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official Steamworks Documentation: https://partner.steamgames.com/doc/api/ISteamParentalSettings
*/

#pragma once

#include "CoreMinimal.h"
#include "SteamCorePro/SteamCoreProModule.h"
#include "SteamParentalSettingsTypes.h"
#include "SteamParentalSettings.generated.h"

UCLASS()
class STEAMCOREPRO_API USteamProParentalSettings : public USteamCoreInterface
{
	GENERATED_BODY()
public:
	USteamProParentalSettings();
	virtual ~USteamProParentalSettings() override;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SteamCore")
	static USteamProParentalSettings* GetSteamParentalSettings();
public:
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
	//		Steam API Functions
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

	/**
	* Is parental lock enabled
	*/
	UFUNCTION(BlueprintCallable, Category = "SteamCore|ParentalSettings")
	bool BIsParentalLockEnabled();

	/**
	* Is parental lock locked
	*/
	UFUNCTION(BlueprintCallable, Category = "SteamCore|ParentalSettings")
	bool BIsParentalLockLocked();

	/**
	* Is app blocked
	*
	* @param	AppID
	*/
	UFUNCTION(BlueprintCallable, Category = "SteamCore|ParentalSettings")
	bool BIsAppBlocked(int32 AppID);

	/**
	* Is app blocked
	*
	* @param	AppID
	*/
	UFUNCTION(BlueprintCallable, Category = "SteamCore|ParentalSettings")
	bool BIsAppInBlockList(int32 AppID);

	/**
	* Is feature blocked
	*
	* @param	Feature		feature
	*/
	UFUNCTION(BlueprintCallable, Category = "SteamCore|ParentalSettings")
	bool BIsFeatureBlocked(ESteamParentalFeature Feature);

	/**
	* Is feature blocked
	*
	* @param	Feature		feature
	*/
	UFUNCTION(BlueprintCallable, Category = "SteamCore|ParentalSettings")
	bool BIsFeatureInBlockList(ESteamParentalFeature Feature);
};
