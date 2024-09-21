// Copyright Samuel Reitich. All rights reserved.

#include "ProjectCrashEditor.h"

#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "AssetTypes/AssetTypeActions_ActionSet.h"
#include "AssetTypes/AssetTypeActions_CrashAbilitySet.h"
#include "AssetTypes/AssetTypeActions_CrashCameraMode.h"
#include "AssetTypes/AssetTypeActions_EquipmentDefinition.h"
#include "AssetTypes/AssetTypeActions_EquipmentSkin.h"
#include "AssetTypes/AssetTypeActions_GameModeData.h"
#include "AssetTypes/AssetTypeActions_InputActionMapping.h"
#include "AssetTypes/AssetTypeActions_PawnData.h"
#include "AssetTypes/AssetTypeActions_Teams.h"
#include "AssetTypes/AssetTypeActions_InventoryItemDefinition.h"
#include "GameFramework/GameFeatures/GameFeatureManager.h"
#include "GameFramework/GameModes/GameplayDebuggerCategory_GameModeProperties.h"
#include "Modules/ModuleManager.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"

#if WITH_GAMEPLAY_DEBUGGER_CORE
#include "GameplayDebugger.h"
#endif // WITH_GAMEPLAY_DEBUGGER_CORE

#if WITH_GAMEPLAY_DEBUGGER
#include "Equipment/GameplayDebuggerCategory_Equipment.h"
#include "Inventory/GameplayDebuggerCategory_Inventory.h"
#endif // WITH_GAMEPLAY_DEBUGGER

#define LOCTEXT_NAMESPACE "ProjectCrashEditorModule"

#define IMAGE_BRUSH_SVG( RelativePath, ... ) FSlateVectorImageBrush(StyleSetInstance->RootToContentDir(RelativePath, TEXT(".svg")), __VA_ARGS__)

DEFINE_LOG_CATEGORY(LogProjectCrashEditor);

struct FClassIconInfo
{
	FClassIconInfo(const char* InType, const char* InIconName) :
		Type(InType),
		IconName(InIconName)
	{}

	/** The name of the class to set an icon and thumbnail for. */
	const char* Type;
	/** The name of the SVG file to use as the class icon and thumbnail. */
	const char* IconName;
};

void FProjectCrashEditorModule::StartupModule()
{
	// Bind callback to when PIE starts.
	if (!IsRunningGame())
	{
		FEditorDelegates::BeginPIE.AddRaw(this, &ThisClass::OnBeginPIE);
	}


	// Register asset categories.
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	GameDataAssetCategory = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("GameData")), LOCTEXT("GameDataAssetCategory", "Game Data"));
	InventoryAssetCategory = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("Inventory")), LOCTEXT("InventoryAssetCategory", "Inventory"));
	SkinAssetCategory = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("Skins")), LOCTEXT("SkinAssetCategory", "Skins"));


	// Register asset types.
	AssetType_ActionSet = MakeShared<FAssetTypeActions_ActionSet>();
	AssetTools.RegisterAssetTypeActions(AssetType_ActionSet.ToSharedRef());

	AssetType_CrashAbilitySet = MakeShared<FAssetTypeActions_CrashAbilitySet>();
	AssetTools.RegisterAssetTypeActions(AssetType_CrashAbilitySet.ToSharedRef());

	AssetType_CrashCameraMode = MakeShared<FAssetTypeActions_CrashCameraMode>();
	AssetTools.RegisterAssetTypeActions(AssetType_CrashCameraMode.ToSharedRef());

	AssetType_EquipmentDefinition = MakeShared<FAssetTypeActions_EquipmentDefinition>();
	AssetTools.RegisterAssetTypeActions(AssetType_EquipmentDefinition.ToSharedRef());

	AssetType_EquipmentSkin = MakeShared<FAssetTypeActions_EquipmentSkin>();
	AssetTools.RegisterAssetTypeActions(AssetType_EquipmentSkin.ToSharedRef());

	AssetType_GameModeData = MakeShared<FAssetTypeActions_GameModeData>();
	AssetTools.RegisterAssetTypeActions(AssetType_GameModeData.ToSharedRef());

	AssetType_InputActionMapping = MakeShared<FAssetTypeActions_InputActionMapping>();
	AssetTools.RegisterAssetTypeActions(AssetType_InputActionMapping.ToSharedRef());

	AssetType_InventoryItemDefinition = MakeShared<FAssetTypeActions_InventoryItemDefinition>();
	AssetTools.RegisterAssetTypeActions(AssetType_InventoryItemDefinition.ToSharedRef());

	AssetType_PawnData = MakeShared<FAssetTypeActions_PawnData>();
	AssetTools.RegisterAssetTypeActions(AssetType_PawnData.ToSharedRef());

	AssetType_TeamCreationComponent = MakeShared<FAssetTypeActions_TeamCreationComponent>();
	AssetTools.RegisterAssetTypeActions(AssetType_TeamCreationComponent.ToSharedRef());

	AssetType_TeamDisplayAsset = MakeShared<FAssetTypeActions_TeamDisplayAsset>();
	AssetTools.RegisterAssetTypeActions(AssetType_TeamDisplayAsset.ToSharedRef());

	AssetType_UserFacingGameModeData = MakeShared<FAssetTypeActions_UserFacingGameModeData>();
	AssetTools.RegisterAssetTypeActions(AssetType_UserFacingGameModeData.ToSharedRef());


	// Create a new style set for custom icons.
	StyleSetInstance = MakeShareable(new FSlateStyleSet("ProjectCrashEditorStyle"));

	// Assign the content root of the style set.
	StyleSetInstance->SetContentRoot(FPaths::ProjectContentDir() / TEXT("Editor/Slate"));

	/* Set icons and thumbnails. To customize a class's asset appearance with an SVG, import the SVG file into the
	 * Content/Editor/Slate directory, and add an entry to this list in the form
	 * {"ClassOrStructName", "FileNameWithoutExtension"}. */
	const FClassIconInfo AssetTypesSVG[] = {
		{"CrashAbilitySet", "CrashAbilitySet"},
		{"CrashAttributeSet", "CrashAttributeSet"},
		{"CrashCameraModeBase", "CrashCameraModeBase"},
		{"CrashCameraModeBlueprint", "CrashCameraModeBase"},
		{"CrashGameModeData", "CrashGameModeData"},
		{"CrashGameplayAbilityBase", "CrashGameplayAbilityBase"},
		{"CrashInputActionMapping", "InputActionMapping"},
		{"EquipmentDefinition", "EquipmentDefinition"},
		{"EquipmentSkin", "EquipmentSkin"},
		{"GameFeatureAction_AddAbilities", "CrashGameplayAbilityBase"},
		{"GameFeatureAction_AddComponents", "ActorComponent"},
		{"GameFeatureAction_AddGameModeProperties", "GameplayTag"},
		{"GameFeatureAction_AddInputActionMapping", "InputActionMapping"},
		{"GameFeatureAction_AddInputMappingContext", "InputMappingContext"},
		{"GameFeatureAction_AddWidgets", "Widget"},
		{"GameFeatureActionSet", "GameFeatureActionSet"},
		{"GameplayCueNotify_Actor", "GameplayCue"},
		{"GameplayCueNotify_Static", "GameplayCue"},
		{"GameplayEffect", "GameplayEffect"},
		{"InventoryItemDefinition", "InventoryItemDefinition"},
		{"InventoryItemDefinitionBlueprint", "InventoryItemDefinition"},
		{"PawnData", "PawnData"},
		{"TeamCreationComponent", "TeamCreationComponent"},
		{"TeamDisplayAsset", "TeamDisplayAsset"},
		{"UserFacingGameModeData", "UserFacingGameModeData"},
	};

	for (int32 TypeIndex = 0; TypeIndex < UE_ARRAY_COUNT(AssetTypesSVG); ++TypeIndex)
	{
		const FClassIconInfo& Info = AssetTypesSVG[TypeIndex];

		// Look up if the brush already exists to audit old vs new icons during development.
		FString ClassIconName = FString::Printf(TEXT("ClassIcon.%hs"), Info.Type);
		if (StyleSetInstance->GetOptionalBrush(*ClassIconName, nullptr, nullptr))
		{
			UE_LOG(LogSlate, Log, TEXT("%s already found."), *ClassIconName);
		}

		// Set the class icon.
		StyleSetInstance->Set(*FString::Printf(TEXT("ClassIcon.%hs"), Info.Type), new IMAGE_BRUSH_SVG(FString::Printf(TEXT("/%hs"), Info.IconName), Icon16x16));
		// Set the class thumbnail.
		StyleSetInstance->Set(*FString::Printf(TEXT("ClassThumbnail.%hs"), Info.Type), new IMAGE_BRUSH_SVG(FString::Printf(TEXT("/%hs"), Info.IconName), Icon64x64));
	}

	// Register the style set.
	FSlateStyleRegistry::RegisterSlateStyle(*StyleSetInstance);

	// Register gameplay debugger categories.
#if WITH_GAMEPLAY_DEBUGGER
	IGameplayDebugger& GameplayDebuggerModule = IGameplayDebugger::Get();
	GameplayDebuggerModule.RegisterCategory("Inventory", IGameplayDebugger::FOnGetCategory::CreateStatic(&FGameplayDebuggerCategory_Inventory::MakeInstance), EGameplayDebuggerCategoryState::EnabledInGame);
	GameplayDebuggerModule.RegisterCategory("Equipment", IGameplayDebugger::FOnGetCategory::CreateStatic(&FGameplayDebuggerCategory_Equipment::MakeInstance), EGameplayDebuggerCategoryState::EnabledInGame);
	GameplayDebuggerModule.RegisterCategory("GameModeProperties", IGameplayDebugger::FOnGetCategory::CreateStatic(&FGameplayDebuggerCategory_GameModeProperties::MakeInstance), EGameplayDebuggerCategoryState::EnabledInGame);
	GameplayDebuggerModule.NotifyCategoriesChanged();
#endif // WITH_GAMEPLAY_DEBUGGER
}

void FProjectCrashEditorModule::OnBeginPIE(bool bIsSimulating)
{
	// Notify the game feature manager subsystem when a PIE session starts.
	UGameFeatureManager* GameFeatureManager = GEngine->GetEngineSubsystem<UGameFeatureManager>();
	check(GameFeatureManager);
	GameFeatureManager->OnPlayInEditorBegin();
}

void FProjectCrashEditorModule::ShutdownModule()
{
	// Unbind callbacks from when PIE starts.
    FEditorDelegates::BeginPIE.RemoveAll(this);


	// Unregister asset types.
	if (!FModuleManager::Get().IsModuleLoaded("AssetTools")) return;
	IAssetTools& AssetTools = IAssetTools::Get();
	AssetTools.UnregisterAssetTypeActions(AssetType_ActionSet.ToSharedRef());
	AssetTools.UnregisterAssetTypeActions(AssetType_CrashAbilitySet.ToSharedRef());
	AssetTools.UnregisterAssetTypeActions(AssetType_CrashCameraMode.ToSharedRef());
	AssetTools.UnregisterAssetTypeActions(AssetType_EquipmentDefinition.ToSharedRef());
	AssetTools.UnregisterAssetTypeActions(AssetType_EquipmentSkin.ToSharedRef());
	AssetTools.UnregisterAssetTypeActions(AssetType_InputActionMapping.ToSharedRef());
	AssetTools.UnregisterAssetTypeActions(AssetType_InventoryItemDefinition.ToSharedRef());
	AssetTools.UnregisterAssetTypeActions(AssetType_GameModeData.ToSharedRef());
	AssetTools.UnregisterAssetTypeActions(AssetType_PawnData.ToSharedRef());
	AssetTools.UnregisterAssetTypeActions(AssetType_TeamCreationComponent.ToSharedRef());
	AssetTools.UnregisterAssetTypeActions(AssetType_TeamDisplayAsset.ToSharedRef());
	AssetTools.UnregisterAssetTypeActions(AssetType_UserFacingGameModeData.ToSharedRef());


	// Unregister the style set.
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSetInstance.Get());
	StyleSetInstance.Reset();


	// Unregister gameplay debugger categories.
#if WITH_GAMEPLAY_DEBUGGER
    if (IGameplayDebugger::IsAvailable())
    {
    	IGameplayDebugger& GameplayDebuggerModule = IGameplayDebugger::Get();
    	GameplayDebuggerModule.UnregisterCategory("Inventory");
    	GameplayDebuggerModule.UnregisterCategory("Equipment");
    	GameplayDebuggerModule.UnregisterCategory("GameModeProperties");
    	GameplayDebuggerModule.NotifyCategoriesChanged();
    }
#endif // WITH_GAMEPLAY_DEBUGGER
}

IMPLEMENT_MODULE(FProjectCrashEditorModule, ProjectCrashEditor);

#undef LOCTEXT_NAMESPACE