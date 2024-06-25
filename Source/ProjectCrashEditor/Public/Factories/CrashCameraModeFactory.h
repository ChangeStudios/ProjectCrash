// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "CrashCameraModeFactory.generated.h"

class UCrashCameraModeBase;

/**
 * UCrashCameraModeBase factory.
 */
UCLASS(HideCategories = Object, MinimalAPI)
class UCrashCameraModeFactory : public UFactory
{
	GENERATED_BODY()

	UCrashCameraModeFactory(const FObjectInitializer& ObjectInitializer);

	/** The type of blueprint that will be created. */
	UPROPERTY(EditAnywhere, Category = "CrashCameraModeFactory")
	TSubclassOf<UCrashCameraModeBase> CameraClass;

	//~ Begin UFactory Interface
	virtual bool ConfigureProperties() override;
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;
	//~ Begin UFactory Interface	
};
