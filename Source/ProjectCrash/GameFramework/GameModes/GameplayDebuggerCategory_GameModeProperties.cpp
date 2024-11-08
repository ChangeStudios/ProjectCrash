// Copyright Samuel Reitich. All rights reserved.

#include "GameplayDebuggerCategory_GameModeProperties.h"

#if WITH_GAMEPLAY_DEBUGGER

#include "GameModePropertySubsystem.h"

FGameplayDebuggerCategory_GameModeProperties::FGameplayDebuggerCategory_GameModeProperties()
{
	SetDataPackReplication<FRepData>(&DataPack);

	NumberFormattingOptions.RoundingMode = HalfFromZero;
	NumberFormattingOptions.MinimumIntegralDigits = 1;
	NumberFormattingOptions.MaximumIntegralDigits = 324;
	NumberFormattingOptions.MinimumFractionalDigits = 1;
	NumberFormattingOptions.MaximumFractionalDigits = 324;
}

void FGameplayDebuggerCategory_GameModeProperties::CollectData(APlayerController* OwnerPC, AActor* DebugActor)
{
	// Collect all registered properties.
	if (UWorld* World = GEngine->GetWorldFromContextObject(DebugActor, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (UGameModePropertySubsystem* GameModePropertySubsystem = World->GetSubsystem<UGameModePropertySubsystem>())
		{
			for (const auto& KVP : GameModePropertySubsystem->GameModeProperties)
			{
				// Serialize properties as tuples of the property name and its value.
				FString PropertyName = (KVP.Key.IsValid() ? KVP.Key.ToString() : "<Invalid Tag!>");
				PropertyName = PropertyName.Replace(TEXT("GameMode.Property."), TEXT(""));
				DataPack.GameModeProperties.Add({ PropertyName, KVP.Value });
			}
		}
	}

}

void FGameplayDebuggerCategory_GameModeProperties::DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext)
{
	// Registered Game Mode Properties:
	CanvasContext.Printf(TEXT("Registered Game Mode Properties:"));

	//			No properties.
	if (DataPack.GameModeProperties.IsEmpty())
	{
		CanvasContext.Printf(TEXT("\t\t{red}No registered properties."));
		return;
	}

	//			Property Name: Value
	for (auto NameAndValue : DataPack.GameModeProperties)
	{
		const FText NumeralText = FText::AsNumber(NameAndValue.Value, &NumberFormattingOptions);
		CanvasContext.Printf(TEXT("\t\t{yellow}%s: {white}%s"), *NameAndValue.Key, *NumeralText.ToString());
	}
}

TSharedRef<FGameplayDebuggerCategory> FGameplayDebuggerCategory_GameModeProperties::MakeInstance()
{
	return MakeShareable(new FGameplayDebuggerCategory_GameModeProperties());
}

void FGameplayDebuggerCategory_GameModeProperties::FRepData::Serialize(FArchive& Ar)
{
	Ar << GameModeProperties;
}

#endif // WITH_GAMEPLAY_DEBUGGER