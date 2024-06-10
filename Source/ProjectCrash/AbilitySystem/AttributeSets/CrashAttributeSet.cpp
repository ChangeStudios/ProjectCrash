// Copyright Samuel Reitich. All rights reserved.


#include "AbilitySystem/AttributeSets/CrashAttributeSet.h"

#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"

UWorld* UCrashAttributeSet::GetWorld() const
{
	// Return this attribute set's outer object's world.
	const UObject* OuterObject = GetOuter();
	check(OuterObject);
	return OuterObject->GetWorld();
}

UCrashAbilitySystemComponent* UCrashAttributeSet::GetCrashAbilitySystemComponent() const
{
	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	return ASC ? Cast<UCrashAbilitySystemComponent>(ASC) : nullptr;
}
