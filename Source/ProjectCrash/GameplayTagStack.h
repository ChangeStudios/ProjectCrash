// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "GameplayTagStack.generated.h"

struct FGameplayTagStackContainer;

/**
 * A "stack" of a gameplay tag. Represents a numbered quantity of the gameplay tag.
 *
 * This structure should not be used directly. An FGameplayTagStackContainer should always be used to wrap stacks.
 */
USTRUCT(BlueprintType)
struct FGameplayTagStack : public FFastArraySerializerItem
{
	GENERATED_BODY()

	/** Default constructor. */
	FGameplayTagStack()
	{}

	/** Constructor that initializes this stack as a quantity of a specified tag. */
	FGameplayTagStack(FGameplayTag InTag, int32 InCount)
	{}

	/** Formats this stack as "Tag (xCount)". */
	FString GetDebugString() const { return FString::Printf(TEXT("%s (x%d)"), *Tag.ToString(), Count); }

private:

	// Stack containers have direct access to the stacks that they contain.
	friend FGameplayTagStackContainer;

	/** The tag of which this structure is a stack of. */
	UPROPERTY()
	FGameplayTag Tag;

	/** The number of tags in this stack. */
	UPROPERTY()
	int32 Count = 0;
};



/**
 * A container for a collection of gameplay tag stacks.
 */
USTRUCT(BlueprintType)
struct FGameplayTagStackContainer : public FFastArraySerializer
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	FGameplayTagStackContainer()
	{}



	// Stack management.

public:

	/** Adds the specified number of tags to the stack of the given tag. Creates a new stack with the given count if
	 * one does not yet exist. */
	void AddTags(FGameplayTag Tag, int32 Count);

	/** Removes the specified number of tags from the stack of the given tag. Removes the stack if its count reaches
	 * 0. */
	void RemoveTags(FGameplayTag Tag, int32 Count);

	/** Returns the number of tags in the specified tag stack. Returns 0 if there is no stack of the given tag. */
	int32 GetTagCount(FGameplayTag Tag) const
	{
		return TagCountMap.FindRef(Tag);
	}

	/** Returns true if this container has a stack of the specified tag. */
	bool ContainsTag(FGameplayTag Tag) const
	{
		return TagCountMap.Contains(Tag);
	}



	// Replication.

/* The fast array serializer only replicates the FFastArraySerializerItem (the Stacks array). To replicate TagCountMap,
 * we have to update it to mirror the changes made to the Stacks array when those changes are replicated. */
public:

	/** Update TagCountMap with removed stacks when replicated. */
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);

	/** Update TagCountMap with added stacks when replicated. */
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);

	/** Update TagCountMap with changed stacks when replicated. */
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);

// Serialization.
public:

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize(Stacks, DeltaParams, *this);
	}



	// Tag stacks.

private:

	/** Internal replicated collection of gameplay tag stacks. */
	UPROPERTY()
	TArray<FGameplayTagStack> Stacks;

	/** A map that mirrors the Stacks array which can be used for accelerated queries. */
	TMap<FGameplayTag, int32> TagCountMap;
};



/**
 * Struct required for fast array serialization.
 */
template<>
struct TStructOpsTypeTraits<FGameplayTagStackContainer> : public TStructOpsTypeTraitsBase2<FGameplayTagStackContainer>
{
	enum
	{
		WithNetDeltaSerializer = true
	};
};