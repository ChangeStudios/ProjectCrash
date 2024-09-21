// Copyright Samuel Reitich. All rights reserved.


#include "AbilitySystem/AbilitySystemUtilitiesLibrary.h"

#include "Kismet/GameplayStatics.h"

bool UAbilitySystemUtilitiesLibrary::HasLineOfSight(const UObject* WorldContextObject, AActor* SourceActor, AActor* TargetActor, ETraceTypeQuery TraceChannel, bool bIgnorePawns)
{
	if (!ensure(SourceActor && TargetActor))
	{
		return false;
	}

	UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);

	// Always ignore the actors performing the LOS trace.
	TArray<AActor*> ActorsToIgnore = { SourceActor, TargetActor };

	// Ignore every pawn in the level, if desired.
	if (bIgnorePawns)
	{
		TArray<AActor*> OutPawns;
		UGameplayStatics::GetAllActorsOfClass(WorldContextObject, APawn::StaticClass(), OutPawns);
		ActorsToIgnore.Append(OutPawns);
	}

	// Collect data for LOS trace.
	FHitResult OutHitResult;
	FVector Start = SourceActor->GetActorLocation();
	FVector End = TargetActor->GetActorLocation();
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActors(ActorsToIgnore);

	// Perform trace.
	World->LineTraceSingleByChannel(OutHitResult, Start, End, UEngineTypes::ConvertToCollisionChannel(TraceChannel));

	// Return if the trace wasn't blocked.
	return !OutHitResult.bBlockingHit;
}
