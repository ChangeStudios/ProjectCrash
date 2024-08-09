// Copyright Samuel Reitich. All rights reserved.


#include "Editors/CrashCameraModeEditor.h"

#if WITH_EDITOR
#include "Editor.h"
#endif

#include "Camera/CrashCameraModeBlueprint.h"

#define LOCTEXT_NAMESPACE "FCrashCameraModeEditor"

void FCrashCameraModeEditor::InitCameraEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, const TArray<UBlueprint*>& InBlueprints, bool bShouldOpenInDefaultsMode)
{
	InitBlueprintEditor(Mode, InitToolkitHost, InBlueprints, bShouldOpenInDefaultsMode);
}

FCrashCameraModeEditor::FCrashCameraModeEditor()
{
}

FCrashCameraModeEditor::~FCrashCameraModeEditor()
{
}

FName FCrashCameraModeEditor::GetToolkitFName() const
{
	return FName("CrashCameraModeEditor");
}

FText FCrashCameraModeEditor::GetBaseToolkitName() const
{
	return LOCTEXT("CrashCameraModeEditorAppLabel", "Camera Mode Editor");
}

FText FCrashCameraModeEditor::GetToolkitName() const
{
	const auto& EditingObjs = GetEditingObjects();

	check(EditingObjs.Num() > 0);

	FFormatNamedArguments Args;

	const UObject* EditingObject = EditingObjs[0];

	const bool bDirtyState = EditingObject->GetOutermost()->IsDirty();

	Args.Add(TEXT("ObjectName"), FText::FromString(EditingObject->GetName()));
	Args.Add(TEXT("DirtyState"), bDirtyState ? FText::FromString(TEXT("*")) : FText::GetEmpty());
	return FText::Format(LOCTEXT("CrashCameraModeToolkitName", "{ObjectName}{DirtyState}"), Args);
}

FText FCrashCameraModeEditor::GetToolkitToolTipText() const
{
	const UObject* EditingObject = GetEditingObject();

	check (EditingObject != NULL);

	return FAssetEditorToolkit::GetToolTipTextForObject(EditingObject);
}

FString FCrashCameraModeEditor::GetWorldCentricTabPrefix() const
{
	return TEXT("CrashCameraModeEditor");
}

FLinearColor FCrashCameraModeEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor::White;
}

FString FCrashCameraModeEditor::GetDocumentationLink() const
{
	return FBlueprintEditor::GetDocumentationLink();
}

UBlueprint* FCrashCameraModeEditor::GetBlueprintObj() const
{
	const TArray<UObject*>& EditingObjs = GetEditingObjects();
	for (int32 i = 0; i < EditingObjs.Num(); ++i)
	{
		if (EditingObjs[i]->IsA<UCrashCameraModeBlueprint>()) 
		{ 
			return (UBlueprint*)EditingObjs[i]; 
		}
	}
	return nullptr;
}
