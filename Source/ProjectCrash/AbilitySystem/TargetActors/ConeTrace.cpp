bool UMathUtilities::SweepConeByChannel(
        UWorld* World,
        TArray <struct FHitResult>& OutHits,
        const FVector& Start,
        const FQuat& Rot,
        const float ConeHeight,
        const float ConeHalfAngle,
        ECollisionChannel TraceChannel,
        const FCollisionQueryParams& Params 
    ) const
{
    check(World);

    TArray<FHitResult> HitResults;

    const FVector End = Start + (Direction * ConeHeight);
    const double ConeHalfAngleRad = FMath::DegreesToRadians(ConeHalfAngle);
    // r = h * tan(theta / 2)
    const double ConeBaseRadius = ConeHeight * tan(ConeHalfAngleRad);
    // s = sqrt(r^2 + h^2)
    const double ConeSlantHeight = FMath::Sqrt((ConeBaseRadius * ConeBaseRadius) + (ConeHeight * ConeHeight));
    const FCollisionShape SphereSweep = FCollisionShape::MakeSphere(ConeBaseRadius);

    // Perform a sweep encompassing an imaginary cone.
    World->SweepMultiByChannel(HitResults, Start, End, Rot, TraceChannel, SphereSweep, Params);

    // Filter for hits that would be inside the cone.
    for (FHitResult& HitResult : HitResults)
    {
        const FVector HitDirection = (HitResult.ImpactPoint - ViewLocation).GetSafeNormal();
        const double Dot = FVector::DotProduct(Rot.Vector(), HitDirection);
        // theta = arccos((A • B) / (|A|*|B|)). |A|*|B| = 1 because A and B are unit vectors.
        const double DeltaAngle = FMath::Acos(Dot);

        // Hit is outside the cone.
        if (DeltaAngle > ConeHalfAngleRad)
        {
            continue;
        }

        // NOTE: HitResult.Distance gives the length of the original sweep, but we want to check the length of the hit.
        const double Distance = (HitResult.ImpactPoint - ViewLocation).Length();
        // Hypotenuse = adjcent / cos(theta)
        const double LengthAtAngle = ConeHeight / cos(DeltaAngle);

        // Hit is beyond the cone. This can happen because we sweep with spheres, which creates a cap at the end of the sweep.
        if (Distance > LengthAtAngle)
        {
            continue;
        }

        OutHits.Add(HitResult);
    }
}