// Copyright Samuel Reitich. All rights reserved.


#include "CrashCameraComponent.h"

#include "CrashCameraModeBase.h"


UCrashCameraComponent::UCrashCameraComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CameraModeStack = nullptr;
}

void UCrashCameraComponent::OnRegister()
{
	Super::OnRegister();

	// Create this component's camera stack, if it doesn't have one yet.
	if (!CameraModeStack)
	{
		CameraModeStack = NewObject<UCrashCameraModeStack>(this);
		check(CameraModeStack);
	}
}

void UCrashCameraComponent::GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView)
{
}

void UCrashCameraComponent::UpdateCameraModes()
{
}

void UCrashCameraComponent::DrawDebug(UCanvas* Canvas) const
{
}
