/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#include "OnlineEncryptedAppTicketInterfaceSteamCore.h"
#include "OnlineSubsystemSteamCorePrivatePCH.h"

enum class ESteamEncryptedAppTicketState
{
	None = 0,
	TicketRequested,
	TicketAvailable,
	TicketFailure 
};

#if WITH_STEAMCORE

class FOnlineAsyncTaskSteamCoreRequestEncryptedAppTicket : public FOnlineAsyncTaskSteamCore
{
public:
	FOnlineAsyncTaskSteamCoreRequestEncryptedAppTicket(FOnlineSubsystemSteamCore* InSteamSubsystem)
		: FOnlineAsyncTaskSteamCore(InSteamSubsystem, k_uAPICallInvalid),
		  m_bInitialized(false), m_CallbackResults()
	{
	}

	virtual FString ToString() const override
	{
		return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreRequestEncryptedAppTicket bWasSuccessful: %d"), WasSuccessful());
	}

	void SetData(TArray<uint8>&& Data)
	{
		m_DataToEncrypt = MoveTemp(Data);
	}

	virtual void Tick() override
	{
		if (!m_bInitialized)
		{
			ISteamUser* const SteamUserPtr = SteamUser();
			if (SteamUserPtr != nullptr)
			{
				const int SizeOfData = m_DataToEncrypt.Num();
				void* const DataPtr = SizeOfData > 0 ? m_DataToEncrypt.GetData() : nullptr;
				m_CallbackHandle = SteamUserPtr->RequestEncryptedAppTicket(DataPtr, SizeOfData);
				m_bInitialized = true;
			}
		}

		if (m_CallbackHandle != k_uAPICallInvalid)
		{
			ISteamUtils* SteamUtilsPtr = SteamUtils();
			check(SteamUtilsPtr);

			bool bFailedCall = false;
			bIsComplete = SteamUtilsPtr->IsAPICallCompleted(m_CallbackHandle, &bFailedCall) ? true : false;
			if (bIsComplete)
			{
				bool bFailedResult;
				bool bSuccessCallResult = SteamUtilsPtr->GetAPICallResult(m_CallbackHandle, &m_CallbackResults, sizeof(m_CallbackResults), m_CallbackResults.k_iCallback, &bFailedResult);
				bWasSuccessful = (bSuccessCallResult ? true : false) &&
					(!bFailedCall ? true : false) &&
					(!bFailedResult ? true : false);
			}
		}
		else
		{
			bWasSuccessful = false;
			bIsComplete = false;
		}
	}

	virtual void Finalize() override
	{
		FOnlineAsyncTaskSteamCore::Finalize();

		if (m_CallbackResults.m_eResult != k_EResultOK)
		{
			bWasSuccessful = false;
		}

		if (!bWasSuccessful)
		{
			LogSteamCoreWarn("Failed to obtain encrypted application ticket, result code: %s", *SteamCore::SteamResultString(m_CallbackResults.m_eResult));
		}
	}

	virtual void TriggerDelegates() override
	{
		FOnlineAsyncTaskSteamCore::TriggerDelegates(); 

		const FOnlineEncryptedAppTicketSteamCorePtr Interface = Subsystem->GetEncryptedAppTicketInterface();
		if (Interface.IsValid())
		{
			Interface->OnAPICallComplete(bWasSuccessful, m_CallbackResults.m_eResult);
		}
	}

private:
	bool m_bInitialized;
	TArray<uint8> m_DataToEncrypt;
	EncryptedAppTicketResponse_t m_CallbackResults;
};

FOnlineEncryptedAppTicketSteamCore::FOnlineEncryptedAppTicketSteamCore(FOnlineSubsystemSteamCore* InSubsystem)
	: FSelfRegisteringExec(),
	  m_SteamSubsystem(InSubsystem),
	  m_TicketState(ESteamEncryptedAppTicketState::None)
{
}

FOnlineEncryptedAppTicketSteamCore::~FOnlineEncryptedAppTicketSteamCore()
{
	m_OnEncryptedAppTicketResultDelegate.Clear();
}

bool FOnlineEncryptedAppTicketSteamCore::RequestEncryptedAppTicket(void* DataToEncrypt, int SizeOfDataToEncrypt)
{
	if (m_TicketState == ESteamEncryptedAppTicketState::TicketRequested)
	{
		LogSteamCoreWarn("Request dropped, prior encrypted application ticket request being processed.");
		return false;
	}

	m_TicketState = ESteamEncryptedAppTicketState::TicketRequested;

	FOnlineAsyncTaskSteamCoreRequestEncryptedAppTicket* NewTask = new FOnlineAsyncTaskSteamCoreRequestEncryptedAppTicket(m_SteamSubsystem);
	if (DataToEncrypt && SizeOfDataToEncrypt > 0)
	{
		TArray<uint8> Data(static_cast<uint8*>(DataToEncrypt), SizeOfDataToEncrypt);
		NewTask->SetData(MoveTemp(Data));
	}

	m_SteamSubsystem->QueueAsyncTask(NewTask);
	return true;
}

void FOnlineEncryptedAppTicketSteamCore::OnAPICallComplete(bool bEncryptedDataAvailable, int32 ResultCode)
{
	m_TicketState = bEncryptedDataAvailable ? ESteamEncryptedAppTicketState::TicketAvailable : ESteamEncryptedAppTicketState::TicketFailure;
	m_OnEncryptedAppTicketResultDelegate.Broadcast(bEncryptedDataAvailable, ResultCode);
}

bool FOnlineEncryptedAppTicketSteamCore::GetEncryptedAppTicket(TArray<uint8>& OutEncryptedData) const
{
	if (m_TicketState == ESteamEncryptedAppTicketState::TicketAvailable)
	{
		if (ISteamUser* const SteamUserPtr = SteamUser())
		{
			uint32 ExactTicketSize = 0;
			SteamUserPtr->GetEncryptedAppTicket(nullptr, 0, &ExactTicketSize);

			if (ExactTicketSize > 0)
			{
				OutEncryptedData.Reset();
				OutEncryptedData.Reserve(ExactTicketSize);
				OutEncryptedData.AddUninitialized(ExactTicketSize);

				if (SteamUserPtr->GetEncryptedAppTicket(OutEncryptedData.GetData(), OutEncryptedData.GetAllocatedSize(), &ExactTicketSize))
				{
					return true;
				}
			}

			LogSteamCoreWarn("Getting encrypted application ticket failed!");
		}
	}
	else
	{
		switch (m_TicketState)
		{
		case ESteamEncryptedAppTicketState::None:
			LogSteamCoreWarn("Unable to get encrypted application ticket, it hasn't been requested!");
			break;

		case ESteamEncryptedAppTicketState::TicketRequested:
			LogSteamCoreWarn("Encrypted ticket is not yet available!");
			break;

		case ESteamEncryptedAppTicketState::TicketFailure:
			LogSteamCoreWarn("Failed to get encrypted application ticket due to the original request failing.");
			break;

		default:
			LogSteamCoreWarn("Failed to get encrypted application ticket, unknown reason.");
			break;
		}
	}

	OutEncryptedData.Reset();
	return false;
}

bool FOnlineEncryptedAppTicketSteamCore::Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar)
{
#if !UE_BUILD_SHIPPING
	if (FParse::Command(&Cmd, TEXT("RequestEncryptedAppTicket")))
	{
		FString StringDataToEncrypt = FParse::Token(Cmd, 0);

		int SizeOfDataToEncrypt = 0;
		void* DataToEncryptPtr = nullptr;
		if (StringDataToEncrypt.GetAllocatedSize() > 0)
		{
			DataToEncryptPtr = &StringDataToEncrypt[0];
			SizeOfDataToEncrypt = StringDataToEncrypt.Len();
		}

		Ar.Log(ELogVerbosity::Display, FString::Printf(TEXT("Requesting encrypted application ticket: DataToEncrypt: %p, SizeOfDataToEncrypt: %d."), DataToEncryptPtr, SizeOfDataToEncrypt));

		const bool bSuccess = RequestEncryptedAppTicket(DataToEncryptPtr, SizeOfDataToEncrypt);

		Ar.Log(ELogVerbosity::Display, FString::Printf(TEXT("Requesting encrypted application ticket %s"), bSuccess ? TEXT("SUCCEEDED.") : TEXT("FAILED!")));

		return true;
	}
	else if (FParse::Command(&Cmd, TEXT("GetEncryptedAppTicket")))
	{
		TArray<uint8> DataBuffer;
		Ar.Log(ELogVerbosity::Display, TEXT("Trying to get encrypted application ticket."));

		if (const bool bSuccess = GetEncryptedAppTicket(DataBuffer))
		{
			const FString RetrievedHexBytes = BytesToHex(DataBuffer.GetData(), DataBuffer.Num());
			Ar.Log(ELogVerbosity::Display, FString::Printf(TEXT("Getting encrypted application ticket SUCCEEDED, encrypted ticket size: %u, data retrieved: %s"), DataBuffer.Num(), *RetrievedHexBytes));
		}
		else
		{
			Ar.Log(ELogVerbosity::Display, TEXT("Getting encrypted application ticket FAILED!"));
		}

		return true;
	}
#endif

	return false;
}
#endif