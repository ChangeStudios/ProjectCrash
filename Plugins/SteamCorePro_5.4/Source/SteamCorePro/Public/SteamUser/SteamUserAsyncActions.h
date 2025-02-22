/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official Steamworks Documentation: https://partner.steamgames.com/doc/api/ISteamUGC
*/

#pragma once

#include "SteamCorePro/SteamCoreProAsync.h"
#include "SteamUserTypes.h"
#include "SteamUserAsyncActions.generated.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		Delegate declarations
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FRequestEncryptedAppTicketDelegate, const FEncryptedAppTicketResponse&, Data, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FRequestStoreAuthURLDelegate, const FStoreAuthURLResponse&, Data, bool, bWasSuccessful);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		USteamCoreProUserAsyncActionRequestEncryptedAppTicket
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
UCLASS()
class STEAMCOREPRO_API USteamCoreProUserAsyncActionRequestEncryptedAppTicket : public USteamCoreProAsyncAction
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FRequestEncryptedAppTicketDelegate OnCallback;
public:
	/**
	* Requests an application ticket encrypted with the secret "encrypted app ticket key".
	*
	* The encryption key can be obtained from the Encrypted App Ticket Key page on the App Admin for your app.
	* There can only be one EncryptedAppTicketResponse_t pending, and this call is subject to a 60 second rate limit.
	* After receiving the response you should call GetEncryptedAppTicket to get the ticket Data, and then you need to send it to a secure server to be decrypted with the SteamEncryptedAppTicket functions.
	*
	* @param	Timeout		How long we wait for this function to finish before aborting
	*/
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", DisplayName = "Request Encrypted App Ticket", AutoCreateRefTerm="DataToInclude"), Category = "SteamCore|User|Async")
	static USteamCoreProUserAsyncActionRequestEncryptedAppTicket* RequestEncryptedAppTicketAsync(UObject* WorldContextObject, TArray<uint8> DataToInclude, float Timeout = 10.f);
public:
	UFUNCTION()
	void HandleCallback(const FEncryptedAppTicketResponse& Data, bool bWasSuccessful);
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		USteamCoreProUserAsyncActionRequestStoreAuthURL
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
UCLASS()
class STEAMCOREPRO_API USteamCoreProUserAsyncActionRequestStoreAuthURL : public USteamCoreProAsyncAction
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FRequestStoreAuthURLDelegate OnCallback;
public:
	/**
	* Requests a URL which authenticates an in-game browser for store check-out, and then redirects to the specified URL.
	*
	* As long as the in-game browser accepts and handles session cookies, Steam microtransaction checkout pages will automatically recognize the user instead of presenting a login page.
	* NOTE: The URL has a very short lifetime to prevent history-snooping attacks, so you should only call this API when you are about to launch the browser, or else immediately navigate to the result URL using a hidden browser window.
	* NOTE: The resulting authorization cookie has an expiration time of one day, so it would be a good idea to request and visit a new auth URL every 12 hours.
	*
	* @param	Timeout		How long we wait for this function to finish before aborting
	*/
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", DisplayName = "Request Store Auth URL"), Category = "SteamCore|User|Async")
	static USteamCoreProUserAsyncActionRequestStoreAuthURL* RequestStoreAuthURLAsync(UObject* WorldContextObject, FString RedirectURL, float Timeout = 10.f);
public:
	UFUNCTION()
	void HandleCallback(const FStoreAuthURLResponse& Data, bool bWasSuccessful);
};
