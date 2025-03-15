// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CrashMathLibrary.generated.h"

/**
 * Math utilities.
 */
UCLASS()
class PROJECTCRASH_API UCrashMathLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/** Returns the intersection of two TArrays. */
	template <typename T>
	static TArray<T> Intersect(const TArray<T>& A, const TArray<T>& B)
	{
		const bool bBSmaller = (A.Num() > B.Num());
		const TArray<T>& a = (bBSmaller ? B : A);
		const TArray<T>& b = (bBSmaller ? A : B);

		TArray<T> Result;
		Result.Reserve(a.Num());

		for (auto ArrIt = a.CreateConstIterator(); ArrIt; ++ArrIt)
		{
			if (b.Contains(*ArrIt))
			{
				Result.AddUnique(*ArrIt);
			}
		}

		return Result;
	};
};
