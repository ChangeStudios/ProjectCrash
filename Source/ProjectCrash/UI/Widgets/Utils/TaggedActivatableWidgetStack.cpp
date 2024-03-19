// Copyright Samuel Reitich 2024.


#include "UI/Widgets/Utils/TaggedActivatableWidgetStack.h"

#include "Player/PlayerControllers/CrashPlayerControllerBase.h"

void UTaggedActivatableWidgetStack::OnWidgetAddedToList(UCommonActivatableWidget& AddedWidget)
{
	Super::OnWidgetAddedToList(AddedWidget);

	// Register this stack with its owning player controller when it is created.
	APlayerController* PC = GetOwningPlayer();
	if (ACrashPlayerControllerBase* CrashPC = PC ? Cast<ACrashPlayerControllerBase>(PC) : nullptr)
	{
		CrashPC->RegisterWidgetStack(this);
	}
}

void UTaggedActivatableWidgetStack::BeginDestroy()
{
	// Unregister this stack from its owning player controller when it is destroyed.
	APlayerController* PC = GetOwningPlayer();
	if (ACrashPlayerControllerBase* CrashPC = PC ? Cast<ACrashPlayerControllerBase>(PC) : nullptr)
	{
		CrashPC->RegisterWidgetStack(this);
	}

	Super::BeginDestroy();
}
