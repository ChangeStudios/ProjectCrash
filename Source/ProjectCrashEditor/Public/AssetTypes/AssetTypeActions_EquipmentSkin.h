// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "AssetTypeActions_Base.h"

/**
 * UEquipmentSkin asset type.
 */
class FAssetTypeActions_EquipmentSkin : public FAssetTypeActions_Base
{
public:
	// IAssetTypeActions Implementation
	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual uint32 GetCategories() override;
	// End IAssetTypeActions Implementation
};
