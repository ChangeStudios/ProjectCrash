// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "Components/ActorComponent.h"
#include "AbilitySystemExtensionComponent.generated.h"

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Behavior_PersistsThroughAvatarDestruction);

class UCrashAbilitySystemComponent;

/**
 * Component used by pawns to manage initialization and uninitialization of its ability system component. This component
 * is controller-agnostic, allowing it to be used for both player characters and AI pawns.
 */
UCLASS()
class PROJECTCRASH_API UAbilitySystemExtensionComponent : public UActorComponent
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	UAbilitySystemExtensionComponent(const FObjectInitializer& ObjectInitializer);



	// Initialization.

protected:

	/** Ensures that this component is being used correctly. */
	virtual void OnRegister() override;

	/** Uninitializes any ability system for which this pawn is the avatar. */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;



	// Utils.

public:

	/** Returns the ability system extension component if one exists on the specified actor. */
	UFUNCTION(BlueprintPure, Category = "Ability System|Extension Component")
	static UAbilitySystemExtensionComponent* FindAbilitySystemExtensionComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<UAbilitySystemExtensionComponent>() : nullptr); }

	/** Gets the current ability system component, which may be owned by a different actor (i.e. by the player state). */
	UFUNCTION(BlueprintPure, Category = "Ability System|Extension Component")
	UCrashAbilitySystemComponent* GetCrashAbilitySystemComponent() const { return AbilitySystemComponent; }

protected:

	/** Pointer to the ability system component, cached for convenience. */
	UPROPERTY()
	TObjectPtr<UCrashAbilitySystemComponent> AbilitySystemComponent;



	// Ability system management.

public:

	/** Called by this component's owning pawn to become the avatar of the given ability system. */
	void InitializeAbilitySystem(UCrashAbilitySystemComponent* InASC, AActor* InOwnerActor);

	/** Called by this component's owning pawn to remove itself as the avatar of its current ability system. */
	void UninitializeAbilitySystem();



	// Pawn events.

public:

	/** Called by this component's owning pawn when its controller changes. Refreshes the ASC's actor info. */
	void HandleControllerChanged();



	// Delegates.

public:

	/** Register with the OnAbilitySystemInitialized delegate and broadcast if our pawn has been registered with the ability system component */
	void OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate Delegate);

	/** Register with the OnAbilitySystemUninitialized delegate fired when our pawn is removed as the ability system's avatar actor */
	void OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate Delegate);

protected:

	/** Delegate fired when this component's owning pawn becomes the avatar of an ability system. */
	FSimpleMulticastDelegate OnAbilitySystemInitialized;

	/** Delegate fired when this component's owning pawn is removed as avatar of an ability system. */
	FSimpleMulticastDelegate OnAbilitySystemUninitialized;
};
