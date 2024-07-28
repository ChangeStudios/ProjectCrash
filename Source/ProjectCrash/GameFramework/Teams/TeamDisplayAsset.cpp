// Copyright Samuel Reitich. All rights reserved.


#include "GameFramework/Teams/TeamDisplayAsset.h"

#include "NiagaraComponent.h"
#include "TeamSubsystem.h"


void UTeamDisplayAsset::ApplyToMaterial(UMaterialInstanceDynamic* Material)
{
	if (Material)
	{
		// Apply scalar properties.
		for (const auto& KVP : Scalars)
		{
			Material->SetScalarParameterValue(KVP.Key, KVP.Value);
		}

		// Apply color properties.
		for (const auto& KVP : Colors)
		{
			Material->SetVectorParameterValue(KVP.Key, FVector(KVP.Value));
		}

		// Apply texture properties.
		for (const auto& KVP : Textures)
		{
			Material->SetTextureParameterValue(KVP.Key, KVP.Value);
		}
	}
}

void UTeamDisplayAsset::ApplyToMeshComponent(UMeshComponent* MeshComponent)
{
	if (MeshComponent)
	{
		// Apply scalar properties to the mesh's materials.
		for (const auto& KVP : Scalars)
		{
			MeshComponent->SetScalarParameterValueOnMaterials(KVP.Key, KVP.Value);
		}

		// Apply color properties to the mesh's materials.
		for (const auto& KVP : Colors)
		{
			MeshComponent->SetVectorParameterValueOnMaterials(KVP.Key, FVector(KVP.Value));
		}

		// Use the material interface to set texture properties.
		const TArray<UMaterialInterface*> MaterialInterfaces = MeshComponent->GetMaterials();
		for (int32 MaterialIndex = 0; MaterialIndex < MaterialInterfaces.Num(); ++MaterialIndex)
		{
			if (UMaterialInterface* MaterialInterface = MaterialInterfaces[MaterialIndex])
			{
				/* Change each of the mesh component's materials to a dynamic material so the texture properties can be
				 * applied. */
				UMaterialInstanceDynamic* DynamicMaterial = Cast<UMaterialInstanceDynamic>(MaterialInterface);
				if (!DynamicMaterial)
				{
					DynamicMaterial = MeshComponent->CreateAndSetMaterialInstanceDynamic(MaterialIndex);
				}

				// Apply texture properties to any new or existing dynamic materials.
				for (const auto& KVP : Textures)
				{
					DynamicMaterial->SetTextureParameterValue(KVP.Key, KVP.Value);
				}
			}
		}
	}
}

void UTeamDisplayAsset::ApplyToNiagaraComponent(UNiagaraComponent* NiagaraComponent)
{
	if (NiagaraComponent)
	{
		// Apply scalar properties.
		for (const auto& KVP : Scalars)
		{
			NiagaraComponent->SetVariableFloat(KVP.Key, KVP.Value);
		}

		// Apply color properties.
		for (const auto& KVP : Colors)
		{
			NiagaraComponent->SetVariableLinearColor(KVP.Key, KVP.Value);
		}

		// Apply texture properties.
		for (const auto& KVP : Textures)
		{
			UTexture* Texture = KVP.Value;
			NiagaraComponent->SetVariableTexture(KVP.Key, Texture);
		}
	}
}

void UTeamDisplayAsset::ApplyToActor(AActor* Actor, bool bIncludeChildActors)
{
	if (Actor)
	{
		Actor->ForEachComponent(bIncludeChildActors, [this](UActorComponent* InComponent)
		{
			// Apply this team's properties to each of the actor's mesh components.
			if (UMeshComponent* MeshComponent = Cast<UMeshComponent>(InComponent))
			{
				ApplyToMeshComponent(MeshComponent);
			}

			// Apply this team's properties to each of the actor's niagara components.
			else if (UNiagaraComponent* NiagaraComponent = Cast<UNiagaraComponent>(InComponent))
			{
				ApplyToNiagaraComponent(NiagaraComponent);
			}
		});
	}
}

#if WITH_EDITOR
void UTeamDisplayAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Notify each PIE instance's team subsystem of the change.
	for (UTeamSubsystem* TeamSubsystem : TObjectRange<UTeamSubsystem>())
	{
		TeamSubsystem->NotifyTeamDisplayAssetModified(this);
	}
}
#endif // WITH_EDITOR