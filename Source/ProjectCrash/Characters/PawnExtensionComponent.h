// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "Components/PawnComponent.h"
#include "PawnExtensionComponent.generated.h"

class UCrashAbilitySystemComponent;
class UPawnData;

/**
 * Component that coordinates initialization between pawn components. This should be added to any pawns that need to
 * handle initialization of their actor components.
 *
 * This component essentially provides a variety of utilities that should be used to initialize a pawn with an ASC, and
 * to initialize any other pawn components.
 *
 * Provides centralized access to the owning pawn's pawn data and ability system component. This allows other classes
 * to use these features without having to know where they are stored. E.g. without having to search the player state
 * AND the pawn for the ASC.
 */
UCLASS()
class PROJECTCRASH_API UPawnExtensionComponent : public UPawnComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	UPawnExtensionComponent(const FObjectInitializer& ObjectInitializer);



	// Initialization.

public:

	/** Ensures only one instance of this component is ever added to a pawn. Registers this component with the
	 * initialization state framework. */
	virtual void OnRegister() override;

	/** Initializes this component's initialization state. */
	virtual void BeginPlay() override;

	/** Uninitializes the owning pawn from its current ASC. Unregisters this component from the initialization state
	 * framework. */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;



	// Initialization states.

public:

	/** The name used to identify this feature (the component) during initialization. */
	static const FName NAME_ActorFeatureName;
	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }

	virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	virtual void CheckDefaultInitialization() override;



	// Pawn data.

public:

	/** Typed getter for this component's current pawn data. */
	template <class T>
	const T* GetPawnData() const { return Cast<T>(PawnData); }

	/** Sets this component's current pawn data. */
	void SetPawnData(const UPawnData* InPawnData);

private:

	/** The pawn data corresponding to this component's owning pawn. Should match the player state's pawn data for
	 * player-controlled pawns. */
	UPROPERTY(EditInstanceOnly, ReplicatedUsing = OnRep_PawnData, Category = "Crash|Pawn")
	TObjectPtr<const UPawnData> PawnData;

	/** Attempts to progress pawn initialization when pawn data is set. */
	UFUNCTION()
	void OnRep_PawnData();



	// Ability system.

// ASC initialization.
public:

	/** Must be called by the owning pawn to become the avatar of the given ASC. */
	void InitializeAbilitySystem(UCrashAbilitySystemComponent* InASC, AActor* InOwnerActor);

	/** Must be called by the owning pawn to remove itself as the avatar of its current ASC. */
	void UninitializeAbilitySystem();

	/** Binds a delegate to when the owning pawn initializes itself as the avatar of an ASC. Immediately invokes the
	 * given delegate if the pawn has already been initialized with an ASC. */
	void OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate Delegate);

	/** Binds a delegate to when the owning pawn removes itself as the avatar of an ASC. */
	void OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate Delegate);

private:

	/** Broadcast when the owning pawn initializes itself as the avatar of an ASC. */
	FSimpleMulticastDelegate OnAbilitySystemInitialized;

	/** Broadcast when the owning pawn removes itself as the avatar of an ASC. */
	FSimpleMulticastDelegate OnAbilitySystemUninitialized;

// Ability system component.
public:

	/** Gets this component's owning pawn's ASC. The ASC may be owned by the pawn, or it may be owned by another actor
	 * (e.g. a player state). */
	UFUNCTION(BlueprintPure, Category = "Crash|Pawn", Meta = (ToolTip = "The owning pawn's ability system component. May be owned by a different actor (e.g. player state)."))
	UCrashAbilitySystemComponent* GetCrashAbilitySystemComponent() const { return AbilitySystemComponent.Get(); }

private:

	/** Owning pawn's ability system component. Cached for convenience. May be owned by another actor (e.g. a player
	 * state). */
	UPROPERTY(Transient)
	TWeakObjectPtr<UCrashAbilitySystemComponent> AbilitySystemComponent;



	// Input.

public:

	
};
