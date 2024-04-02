// Copyright Samuel Reitich 2024.


#include "Characters/ChallengerData.h"

UChallengerData::UChallengerData(const FObjectInitializer& ObjectInitializer)
{
	DefaultAbilitySet = nullptr;
	HealthAttributeBaseValues = nullptr;
	DefaultEquipmentSet = nullptr;
	DefaultAnimData = nullptr;
	DefaultActionMapping = nullptr;
	ChallengerClass = EChallengerClass::Brawler;
	ProfileImage = nullptr;
}
