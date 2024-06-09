// Copyright Samuel Reitich 2024.


#include "Development/CrashDeveloperSettings.h"

#include "Engine/AssetManager.h"


UCrashDeveloperSettings::UCrashDeveloperSettings()
{
}

FName UCrashDeveloperSettings::GetCategoryName() const
{
	// Use the project's name as the category of these developer settings.
	return FApp::GetProjectName();
}

#if WITH_EDITOR
bool UCrashDeveloperSettings::CanEditChange(const FProperty* InProperty) const
{
	bool bIsMutable = Super::CanEditChange(InProperty);

	if (bIsMutable && InProperty != NULL)
	{
		const FName PropName = InProperty->GetFName();

		/* Only allow skin data to be edited if there is an active Challenger override, so the skin data being used by
		 * each player will match their Challenger. */
		if (PropName == GET_MEMBER_NAME_CHECKED(UCrashDeveloperSettings, SkinDataOverride))
		{
			bIsMutable = !UAssetManager::Get().GetPrimaryAssetPath(ChallengerDataOverride).IsNull();
		}
	}

	return bIsMutable;
}
#endif // WITH_EDITOR