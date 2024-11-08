// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Characters/CrashCharacter.h"
#include "CharacterWithAbilities.generated.h"

/**
 * A character with a self-contained ability system component. This is useful for AI-controlled characters that don't
 * want a player state. Should not be used by players; they have their own ability system components contained in their
 * player state.
 *
 * Note that this character will not behave properly if it's spawned with a player state: it won't know which ASC to
 * use.
 */
UCLASS()
class PROJECTCRASH_API ACharacterWithAbilities : public ACrashCharacter
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	ACharacterWithAbilities(const FObjectInitializer& ObjectInitializer);



	// Ability system.

public:

	/** Interfaced getter to retrieve this character's self-contained ASC. */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:

	/** This character's self-contained ability system component. This still relies on the pawn extension component
	 * for initialization. */
	UPROPERTY(VisibleAnywhere, Category = "Crash|Character")
	TObjectPtr<UCrashAbilitySystemComponent> AbilitySystemComponent;



	// Validation.

public:

#if WITH_EDITOR
	/** Makes sure this character is not spawned with a player state. */
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif // WITH_EDITOR
};
