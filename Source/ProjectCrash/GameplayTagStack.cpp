// Copyright Samuel Reitich. All rights reserved.


#include "GameplayTagStack.h"

void FGameplayTagStackContainer::AddTags(FGameplayTag Tag, int32 Count)
{
	// Make sure the given tag is valid.
	if (!Tag.IsValid())
	{
		FFrame::KismetExecutionMessage(TEXT("Attempted to add an invalid gameplay tag to a gameplay tag stack container."), ELogVerbosity::Warning);
		return;
	}

	// Ignore requests to add less than 1 tag.
	if (Count > 0)
	{
		// Search for an existing stack of the given tag.
		for (FGameplayTagStack& Stack : Stacks)
		{
			// If this container has a stack of the given tag, add the given count.
			if (Stack.Tag == Tag)
			{
				const int32 NewCount = Stack.Count + Count;
				Stack.Count = NewCount;
				MarkItemDirty(Stack);

				TagCountMap[Tag] = NewCount;

				return;
			}
		}

		// If this container doesn't have a stack of the given tag, create one.
		FGameplayTagStack& NewStack = Stacks.Emplace_GetRef(Tag, Count);
		MarkItemDirty(NewStack);

		TagCountMap.Add(Tag, Count);
	}
}

void FGameplayTagStackContainer::RemoveTags(FGameplayTag Tag, int32 Count)
{
	// Make sure the given tag is valid.
	if (!Tag.IsValid())
	{
		FFrame::KismetExecutionMessage(TEXT("Attempted to remove an invalid gameplay tag from a gameplay tag stack container."), ELogVerbosity::Warning);
		return;
	}

	// Ignore requests to remove less than 1 tag.
	if (Count > 0)
	{
		// Search for the stack of the given tag.
		for (auto It = Stacks.CreateIterator(); It; ++It)
		{
			FGameplayTagStack& Stack = *It;
			if (Stack.Tag == Tag)
			{
				const int32 NewCount = Stack.Count - Count;

				// If the stack's count reaches 0, remove the stack.
				if (NewCount < 1)
				{
					It.RemoveCurrent();
					MarkArrayDirty();

					TagCountMap.Remove(Tag);
				}
				// Update the stack if it still has at least one tag.
				else
				{
					Stack.Count = NewCount;
					MarkItemDirty(Stack);

					TagCountMap[Tag] = NewCount;
				}

				return;
			}
		}
	}
}

void FGameplayTagStackContainer::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	// When the removal of a stack from the Stacks array is replicated, mirror the removal in TagCountMap.
	for (int32 Index : RemovedIndices)
	{
		const FGameplayTag Tag = Stacks[Index].Tag;
		TagCountMap.Remove(Tag);
	}
}

void FGameplayTagStackContainer::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	// When the addition of a stack from the Stacks array is replicated, mirror the addition in TagCountMap.
	for (int32 Index : AddedIndices)
	{
		const FGameplayTagStack& Stack = Stacks[Index];
		TagCountMap.Add(Stack.Tag, Stack.Count);
	}
}

void FGameplayTagStackContainer::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	// When a change to a stack in the Stacks array is replicated, mirror the change in TagCountMap.
	for (int32 Index : ChangedIndices)
	{
		const FGameplayTagStack& Stack = Stacks[Index];
		TagCountMap[Stack.Tag] = Stack.Count;
	}
}

TMap<FString, int32> FGameplayTagStackContainer::Debug_GetTagStacks() const
{
	TMap<FString, int32> DebugStacks;

	// Compile every tag stack into the debug map.
	for (auto It = Stacks.CreateConstIterator(); It; ++It)
	{
		const FGameplayTagStack& Stack = *It;
		if (Stack.Tag.IsValid())
		{
			DebugStacks.Add(Stack.Tag.ToString(), Stack.Count);
		}
	}

	return DebugStacks;
}
