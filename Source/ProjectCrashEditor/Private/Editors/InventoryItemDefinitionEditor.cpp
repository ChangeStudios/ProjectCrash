// Copyright Samuel Reitich. All rights reserved.


#include "Editors/InventoryItemDefinitionEditor.h"

#if WITH_EDITOR
#include "Editor.h"
#endif

#include "Inventory/InventoryItemDefinitionBlueprint.h"

#define LOCTEXT_NAMESPACE "FInventoryItemDefinitionEditor"

void FInventoryItemDefinitionEditor::InitItemEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, const TArray<UBlueprint*>& InBlueprints, bool bShouldOpenInDefaultsMode)
{
	InitBlueprintEditor(Mode, InitToolkitHost, InBlueprints, bShouldOpenInDefaultsMode);
}

FInventoryItemDefinitionEditor::FInventoryItemDefinitionEditor()
{
}

FInventoryItemDefinitionEditor::~FInventoryItemDefinitionEditor()
{
}

FName FInventoryItemDefinitionEditor::GetToolkitFName() const
{
	return FName("InventoryItemDefinitionEditor");
}

FText FInventoryItemDefinitionEditor::GetBaseToolkitName() const
{
	return LOCTEXT("InventoryItemDefinitionEditorAppLabel", "Inventory Item Editor");
}

FText FInventoryItemDefinitionEditor::GetToolkitName() const
{
	const auto& EditingObjs = GetEditingObjects();

	check(EditingObjs.Num() > 0);

	FFormatNamedArguments Args;

	const UObject* EditingObject = EditingObjs[0];

	const bool bDirtyState = EditingObject->GetOutermost()->IsDirty();

	Args.Add(TEXT("ObjectName"), FText::FromString(EditingObject->GetName()));
	Args.Add(TEXT("DirtyState"), bDirtyState ? FText::FromString(TEXT("*")) : FText::GetEmpty());
	return FText::Format(LOCTEXT("InventoryItemDefinitionToolkitName", "{ObjectName}{DirtyState}"), Args);
}

FText FInventoryItemDefinitionEditor::GetToolkitToolTipText() const
{
	const UObject* EditingObject = GetEditingObject();

	check (EditingObject != NULL);

	return FAssetEditorToolkit::GetToolTipTextForObject(EditingObject);
}

FString FInventoryItemDefinitionEditor::GetWorldCentricTabPrefix() const
{
	return TEXT("InventoryItemDefinitionEditor");
}

FLinearColor FInventoryItemDefinitionEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor::White;
}

FString FInventoryItemDefinitionEditor::GetDocumentationLink() const
{
	return FBlueprintEditor::GetDocumentationLink();
}

UBlueprint* FInventoryItemDefinitionEditor::GetBlueprintObj() const
{
	const TArray<UObject*>& EditingObjs = GetEditingObjects();
	for (int32 i = 0; i < EditingObjs.Num(); ++i)
	{
		if (EditingObjs[i]->IsA<UInventoryItemDefinitionBlueprint>()) 
		{ 
			return (UBlueprint*)EditingObjs[i]; 
		}
	}
	return nullptr;
}

#undef LOCTEXT_NAMESPACE
