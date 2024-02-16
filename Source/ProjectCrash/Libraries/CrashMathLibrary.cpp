// Copyright Samuel Reitich 2024.


#include "Libraries/CrashMathLibrary.h"

FVector UCrashMathLibrary::RandomVectorInRange(const FVector& Min, const FVector& Max)
{
	return FVector
	(
		FMath::RandRange(Min.X, Max.X),
		FMath::RandRange(Min.Y, Max.Y),
		FMath::RandRange(Min.Z, Max.Z)
	);
}
