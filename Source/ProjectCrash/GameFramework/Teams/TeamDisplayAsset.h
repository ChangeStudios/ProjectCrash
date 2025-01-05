// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TeamDisplayAsset.generated.h"

/**
 * Defines cosmetic information related to a team (colors, textures, etc.).
 */
UCLASS(BlueprintType)
class PROJECTCRASH_API UTeamDisplayAsset : public UDataAsset
{
	GENERATED_BODY()

public:

	/** The user-facing name of this team (usually just the color, e.g. "Blue"). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName = "User-Facing Team Name")
	FText TeamName;

	/** Team scalar properties. E.g. "fresnel strength." */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName = "Scalar Properties")
	TMap<FName, float> Scalars;

	/** Team color properties. E.g. "team UI color." */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName = "Color Properties")
	TMap<FName, FLinearColor> Colors;

	/** Team texture properties. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName = "Texture Properties")
	TMap<FName, TObjectPtr<UTexture>> Textures;

public:

	/** Applies this team's properties to the given material. */
	UFUNCTION(BlueprintCallable, Category = "Teams")
	void ApplyToMaterial(UMaterialInstanceDynamic* Material);

	/** Applies this team's properties to the given mesh's materials. */
	UFUNCTION(BlueprintCallable, Category = "Teams")
	void ApplyToMeshComponent(UMeshComponent* MeshComponent);

	/** Applies this team's properties to the given niagara system's parameters. */
	UFUNCTION(BlueprintCallable, Category = "Teams")
	void ApplyToNiagaraComponent(UNiagaraComponent* NiagaraComponent);

	/** Applies this team's properties to each of the given actor's mesh and niagara components. */
	UFUNCTION(BlueprintCallable, Category = "Teams")
	void ApplyToActor(AActor* Actor, bool bIncludeChildActors = true);



	// Utils.

public:

#if WITH_EDITOR

	/** Broadcasts changes to this display asset. Allows display assets to be modified in PIE. */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

#endif // WITH_EDITOR
};
