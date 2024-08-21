// Copyright Samuel Reitich. All rights reserved.


#include "AssetTypes/AssetTypeActions_CrashCameraMode.h"

#include "Camera/CrashCameraModeBase.h"
#include "Camera/CrashCameraModeBlueprint.h"
#include "Editors/CrashCameraModeEditor.h"
#include "Factories/CrashCameraModeFactory.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

FText FAssetTypeActions_CrashCameraMode::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_CrashCameraMode", "Crash Camera Mode");
}

FColor FAssetTypeActions_CrashCameraMode::GetTypeColor() const
{
	return FColor(255, 255, 25);
}

UClass* FAssetTypeActions_CrashCameraMode::GetSupportedClass() const
{
	return UCrashCameraModeBlueprint::StaticClass();
}

void FAssetTypeActions_CrashCameraMode::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		auto Blueprint = Cast<UBlueprint>(*ObjIt);
		if (Blueprint )
		{
			bool bLetOpen = true;
			if (!Blueprint->ParentClass)
			{
				bLetOpen = EAppReturnType::Yes == FMessageDialog::Open(
					EAppMsgType::YesNo, 
					LOCTEXT("FailedToLoadCameraBlueprintWithContinue", "Camera Mode Blueprint could not be loaded because it derives from an invalid class.  Check to make sure the parent class for this blueprint hasn't been removed! Do you want to continue (it can crash the editor)?")
				);
			}
		
			if (bLetOpen)
			{
				TSharedRef< FCrashCameraModeEditor > NewEditor(new FCrashCameraModeEditor());

				TArray<UBlueprint*> Blueprints;
				Blueprints.Add(Blueprint);

				NewEditor->InitCameraEditor(Mode, EditWithinLevelEditor, Blueprints, true);
			}
		}
		else
		{
			FMessageDialog::Open( EAppMsgType::Ok, LOCTEXT("FailedToLoadCameraBlueprint", "Camera Mode Blueprint could not be loaded because it derives from an invalid class.  Check to make sure the parent class for this blueprint hasn't been removed!"));
		}
	}}

uint32 FAssetTypeActions_CrashCameraMode::GetCategories()
{
	return EAssetTypeCategories::Gameplay;
}

UFactory* FAssetTypeActions_CrashCameraMode::GetFactoryForBlueprintType(UBlueprint* InBlueprint) const
{
	UCrashCameraModeFactory* CrashCameraModeFactory = NewObject<UCrashCameraModeFactory>();
	CrashCameraModeFactory->CameraClass = UCrashCameraModeBase::StaticClass();
	return CrashCameraModeFactory;
}

#undef LOCTEXT_NAMESPACE