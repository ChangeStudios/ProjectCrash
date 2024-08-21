// Copyright Samuel Reitich. All rights reserved.


#include "Factories/CrashCameraModeFactory.h"

#include "ClassViewerFilter.h"
#include "ClassViewerModule.h"
#include "Camera/CrashCameraModeBase.h"
#include "Camera/CrashCameraModeBlueprint.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/SClassPickerDialog.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

/**
 * Helper struct that filters classes for class selection prompts.
 */
class FAssetClassParentFilter : public IClassViewerFilter
{
public:
	FAssetClassParentFilter()
		: DisallowedClassFlags(CLASS_None), bDisallowBlueprintBase(false)
	{}

	/** All children of these classes will be included unless filtered out by another setting. */
	TSet< const UClass* > AllowedChildrenOfClasses;

	/** Disallowed class flags. */
	EClassFlags DisallowedClassFlags;

	/** Disallow blueprint base classes. */
	bool bDisallowBlueprintBase;

	virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs) override
	{
		bool bAllowed= !InClass->HasAnyClassFlags(DisallowedClassFlags)
			&& InClass->CanCreateAssetOfClass()
			&& InFilterFuncs->IfInChildOfClassesSet(AllowedChildrenOfClasses, InClass) != EFilterReturn::Failed;

		if (bAllowed && bDisallowBlueprintBase)
		{
			if (FKismetEditorUtilities::CanCreateBlueprintOfClass(InClass))
			{
				return false;
			}
		}

		return bAllowed;
	}

	virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef< const IUnloadedBlueprintData > InUnloadedClassData, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs) override
	{
		if (bDisallowBlueprintBase)
		{
			return false;
		}

		return !InUnloadedClassData->HasAnyClassFlags(DisallowedClassFlags)
			&& InFilterFuncs->IfInChildOfClassesSet(AllowedChildrenOfClasses, InUnloadedClassData) != EFilterReturn::Failed;
	}
};

UCrashCameraModeFactory::UCrashCameraModeFactory(const FObjectInitializer& ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UCrashCameraModeBlueprint::StaticClass();
	CameraClass = UCrashCameraModeBase::StaticClass();
}

bool UCrashCameraModeFactory::ConfigureProperties()
{
	CameraClass = nullptr;

	// Load the class viewer module to display a class selector prompt.
	FClassViewerModule& ClassViewerModule = FModuleManager::LoadModuleChecked<FClassViewerModule>("ClassViewer");

	// Fill in the class selector's options.
	FClassViewerInitializationOptions Options;
	Options.Mode = EClassViewerMode::ClassPicker;
	Options.NameTypeToDisplay = EClassViewerNameTypeToDisplay::DisplayName;
	TSharedPtr<FAssetClassParentFilter> Filter = MakeShareable(new FAssetClassParentFilter);
	Options.ClassFilters.Add(Filter.ToSharedRef());

	Filter->DisallowedClassFlags = CLASS_Deprecated | CLASS_NewerVersionExists | CLASS_HideDropDown | CLASS_CompiledFromBlueprint;
	Filter->AllowedChildrenOfClasses.Add(UCrashCameraModeBase::StaticClass());

	// Display class selection prompt.
	const FText TitleText = LOCTEXT("CreateCameraOptions", "Pick Camera Mode Class");
	UClass* ChosenClass = nullptr;
	const bool bPressedOk = SClassPickerDialog::PickClass(TitleText, Options, ChosenClass, UCrashCameraModeBase::StaticClass());

	if ( bPressedOk )
	{
		CameraClass = ChosenClass;
	}

	return bPressedOk;
}

UObject* UCrashCameraModeFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	check(InClass->IsChildOf(UCrashCameraModeBlueprint::StaticClass()));

	if ( ( CameraClass == NULL ) || !FKismetEditorUtilities::CanCreateBlueprintOfClass(CameraClass) || !CameraClass->IsChildOf(UCrashCameraModeBase::StaticClass()) )
	{
		FFormatNamedArguments Args;
		Args.Add( TEXT("ClassName"), (CameraClass != NULL) ? FText::FromString( CameraClass->GetName() ) : LOCTEXT("Null", "(null)") );
		FMessageDialog::Open( EAppMsgType::Ok, FText::Format( LOCTEXT("CannotCreateCameraMode", "Cannot create a Camera Mode based on the class '{ClassName}'."), Args ) );
		return NULL;
	}
	else
	{
		UCrashCameraModeBlueprint* NewBP = CastChecked<UCrashCameraModeBlueprint>(FKismetEditorUtilities::CreateBlueprint(CameraClass, InParent, InName, BPTYPE_Normal, UCrashCameraModeBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass(), CallingContext));
		return NewBP;
	}
}

UObject* UCrashCameraModeFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return FactoryCreateNew(InClass, InParent, InName, Flags, Context, Warn, NAME_None);
}

#undef LOCTEXT_NAMESPACE
