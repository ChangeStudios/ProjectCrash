// Copyright Samuel Reitich 2024.


#include "GameFramework/GameModes/Game/CrashGM_Deathmatch.h"

void ACrashGM_Deathmatch::StartDeath(AActor* DyingActor, UAbilitySystemComponent* DyingActorASC, AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec& DamageEffectSpec)
{
	Super::StartDeath(DyingActor, DyingActorASC, DamageInstigator, DamageCauser, DamageEffectSpec);
}
