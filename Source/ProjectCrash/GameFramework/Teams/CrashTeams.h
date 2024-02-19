// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "CrashTeams.generated.h"

/**
 * Defines a team member's attitude towards another team member.
 */
UENUM(BlueprintType)
enum ECrashTeamAttitude : uint8
{
	Friendly,
	Neutral,
	Hostile
};



/**
 * Represents a certain team. Provides utilities for finding and comparing teams and team members.
 *
 * Any actors that use this type as a member to represent their team must implement ICrashTeamMemberInterface to
 * properly use it.
 */
USTRUCT(BlueprintType)
struct FCrashTeamID
{
	GENERATED_BODY()

	// Initialization.

public:

	/** Default constructor. */
	FCrashTeamID(uint8 InTeamID = NO_TEAM)
		: TeamID(InTeamID)
	{}



	// Team ID.

// ID.
protected:

	/** The ID of the team that this structure represents. Core value of the FCrashTeamID structure. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teams")
	uint8 TeamID;

// No team.
public:

	/** An ID reserved for players with no team. Anything with its team set to NO_TEAM will be considered "Neutral" to
	 * other teams. */
	static PROJECTCRASH_API const uint8 NO_TEAM = 255;



	// Utils.

public:

	/** Allows this structure to be referred to directly by its TeamID value. Since this is public, we don't need a
	 * getter for TeamID. */
	FORCEINLINE operator uint8() const { return TeamID; }

	/** Retrieves an actor's team using ICrashTeamMemberInterface. Returns NO_TEAM if the given actor does not
	 * implement the interface. */
	static FCrashTeamID GetActorTeamID(const AActor* TeamMember);

	/** Returns TeamMemberA's attitude towards TeamMemberB. Returns Neutral if either actor does not implement
	 * ICrashTeamMemberInterface. Wraps GetAttitude. */
	static ECrashTeamAttitude GetAttitude(const AActor* TeamMemberA, const AActor* TeamMemberB);

	/** Returns TeamA's attitude towards TeamB. By default, different teams are Hostile, matching teams are Friendly,
	 * and NO_TEAM is Neutral. I might change this to a function pointer in the future to allow for custom
	 * implementations. */
	static ECrashTeamAttitude GetAttitude(FCrashTeamID TeamA, FCrashTeamID TeamB);
};



/**
 * Interface for any actor that will be a member of a team. Provides utilities for easier interactions between team
 * members.
 */
UINTERFACE(MinimalAPI)
class UCrashTeamMemberInterface : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

class ICrashTeamMemberInterface
{
	GENERATED_IINTERFACE_BODY()

public:

	/** Retrieves this actor's current team ID. */
	virtual FCrashTeamID GetTeamID() const { return FCrashTeamID::NO_TEAM; }

	/** Returns this team member's attitude toward a given team member. Returns Neutral if the given actor does not
	 * implement ICrashTeamMemberInterface. */
	virtual ECrashTeamAttitude GetAttitudeTowards(const AActor* Other) const
	{
		const ICrashTeamMemberInterface* OtherTeamMember = Cast<const ICrashTeamMemberInterface>(Other);

		// If the other team member does not implement this interface, return Neutral.
		if (!OtherTeamMember)
		{
			return ECrashTeamAttitude::Neutral;
		}

		return FCrashTeamID::GetAttitude(GetTeamID(), OtherTeamMember->GetTeamID());
	}
};