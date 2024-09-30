// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "AbilitySystem/Abilities/CrashAbilitySet.h"
#include "CoreMinimal.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "Components/PawnComponent.h"
#include "PawnExtensionComponent.generated.h"

class UCrashAbilitySystemComponent;
class UPawnData;

/**
 * Component that coordinates initialization between pawn components. This should be added to any pawns that need to
 * manage initialization of their actor components. This can be used by both player and non-player pawns.
 *
 * This component provides a variety of utilities for initializing a pawn's components, such as centralized access to
 * the owning pawn's pawn data.
 *
 * This also provides centralized access to the pawn's ASC, allowing other classes to access it without having to know
 * where it's stored (e.g. player state vs pawn). This component also handles automatic initialization of the ASC,
 * either using the pawn's ASC (if it stores one itself, i.e. non-player pawns) or using the pawn's owning player
 * state, if it has one.
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



	// Pawn events.

public:

	/** Must be called by the owning pawn when its controller changes. Refreshes ASC actor info and checks init state
	 * progress. */
	void HandleControllerChanged();

	/** Must be called by the owning pawn when its owning player state has been replicated. Checks init state
	 * progress. */
	void HandlePlayerStateReplicated();

	/** Must be called by the owning pawn when its input component finishes setting up. Checks init state progress. */
	void HandleInputComponentSetUp();



	// Pawn data.

public:

	/** Typed getter for this component's current pawn data. */
	template <class T>
	const T* GetPawnData() const { return Cast<T>(PawnData); }

	/**
	 * Must be called when this pawn is created to set its pawn data.
	 *
	 * By default, this is called by the game mode when spawning a new pawn. But this could also be called, for
	 * example, by a bot-spawner, after spawning a bot.
	 */
	void SetPawnData(const UPawnData* InPawnData);

private:

	/** The pawn data corresponding to this component's owning pawn. Can be set in-editor for pawns that are not
	 * dynamically spawned. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_PawnData, Category = "Crash|Pawn", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<const UPawnData> PawnData;

	/** Attempts to progress pawn initialization when pawn data is set. */
	UFUNCTION()
	void OnRep_PawnData();



	// Ability system.

// ASC initialization.
public:

	/** Called by the owning pawn to become the avatar of the given ASC. Called automatically by the pawn extension
	 * component when it enters the Initializing state. */
	void InitializeAbilitySystem(UCrashAbilitySystemComponent* InASC, AActor* InOwnerActor);

	/** Called by the owning pawn to remove itself as the avatar of its current ASC, usually when it's destroyed. */
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



	// Utils.

public:

	/** Retrieves the given actor's pawn extension component, if it has one. Returns nullptr otherwise. */
	UFUNCTION(BlueprintPure, Category = "Crash|Pawn")
	static UPawnExtensionComponent* FindPawnExtensionComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<UPawnExtensionComponent>() : nullptr); }
};
