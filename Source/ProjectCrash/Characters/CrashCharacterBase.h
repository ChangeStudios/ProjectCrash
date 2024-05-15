// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Character.h"
#include "CrashCharacterBase.generated.h"


/**
 * The perspective through which a character can be viewed.
 */
UENUM(BlueprintType)
enum class ECharacterPerspective : uint8
{
	FirstPerson,
	ThirdPerson
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FJumpedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPerspectiveChangedSignature, FGameplayTag, NewPerspective);

/**
 * Base class for characters in this project. Contains universal logic used by all characters, such as custom
 * delegates.
 */
UCLASS()
class PROJECTCRASH_API ACrashCharacterBase : public ACharacter
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	ACrashCharacterBase(const FObjectInitializer& ObjectInitializer);



	// Initialization.

public:

	/** Initializes perspective on the server. */
	virtual void PossessedBy(AController* NewController) override;

	/** Initializes perspective on the client. */
	virtual void OnRep_Controller() override;

	/** Additional perspective initialization for safety. */
	virtual void PostInitializeComponents() override;



	// Components.

public:

	/** Accessor for this character's first-person camera component. */
	UFUNCTION(Category = "Character|Components")
	virtual UCameraComponent* GetFirstPersonCameraComponent() const { return nullptr; }

	/** Accessor for this character's third-person camera component. */
	UFUNCTION(Category = "Character|Components")
	virtual UCameraComponent* GetThirdPersonCameraComponent() const { return nullptr; }

	/** Returns this character's first-person-only mesh, if it has one. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Characters|Components")
	virtual USkeletalMeshComponent* GetFirstPersonMesh() const { return nullptr; }

	/** Returns this character's third-person-only mesh. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Characters|Components")
	virtual USkeletalMeshComponent* GetThirdPersonMesh() const { return GetMesh(); }



	// Perspective.

public:

	/** Getter for this character's current local perspective. */
	UFUNCTION(BlueprintPure, Category = "Character|Perspective")
	FGameplayTag GetCurrentPerspective() const { return CharacterPerspective; }

	/** Sets this character's local perspective, which will switch to the appropriate meshes and camera. */
	UFUNCTION(BlueprintCallable, Category = "Character|Perspective", Meta = (Categories = "State.Perspective"))
	void SetPerspective(FGameplayTag NewPerspective);

	/** Delegate fired when this character's local perspective changes. */
	UPROPERTY(BlueprintAssignable)
	FPerspectiveChangedSignature PerspectiveChangedDelegate;

protected:

	/** Initializes this character's perspective using its local role. */
	void InitializePerspective();

	/** Determines whether we can enter first-person on this character. We must have a valid first-person mesh and
	 * camera to use. */
	FORCEINLINE virtual bool CanEnterFirstPerson() const;

	/** The perspective from which this character is being locally viewed. Determines which meshes are visible and
	 * which camera is used. By default, all characters support third-person viewing. For first-person viewing, the
	 * character must have a first-person camera and mesh defined. */
	UPROPERTY(Meta = (Categories = "State.Perspective"))
	FGameplayTag CharacterPerspective;



	// Perspective events.

protected:

	/** Begins listening for */
	void ListenForPerspectiveStates();

	UFUNCTION()
	void OnPerspectiveStateChanged(const FGameplayTag Tag, int32 NewCount);

private:

	/** Total number of current perspective tags. */
	uint32 TotalPerspectiveTags;

	/** Current perspective states. */
	TMap<const FGameplayTag, uint32> PerspectiveTagsStacks;

	/** The perspective that this character was in before it received its first perspective state tag, which it will
	 * return to when all perspective tags have been removed. */
	FGameplayTag BackingCharacterPerspective;



	// Movement.

// Jumping.
public:

	/** Broadcasts JumpedDelegate. */
	virtual void OnJumped_Implementation() override;

	/** Called when this character successfully jumps. */
	UPROPERTY(BlueprintAssignable, Category = Character)
	FJumpedSignature JumpedDelegate;

};
