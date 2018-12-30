#pragma once

#include <stdlib.h>

class StringHashMap
{
public:
	StringHashMap()
	{}

	~StringHashMap()
	{
		StringHashNode *Next, *HashNode;
		int Index = BucketCount;

		while (Index--)
		{
			HashNode = Buckets[Index];

			while (HashNode)
			{
				Next = HashNode->Next;
				free(HashNode);
				HashNode = Next;
			}
		}

		free(Buckets);
	}

	BOOL Retrieve(const char *Key, int *Output = NULL)
	{
		StringHashNode **Result = GetReference(Key);

		if (Result)
		{
			if (Output)
				*Output = (*Result)->Value;

			return TRUE;
		}

		return FALSE;
	}

	int Insert(const char *Key, int Value)
	{
		int Index;
		StringHashNode **NextHashNode, *HashNode;

		NextHashNode = GetReference(Key);

		if (NextHashNode)
		{
			(*NextHashNode)->Value = Value;
			return 0;
		}

		HashNode = CreateNode(Key, Value);

		if (HashNode == NULL)
			goto Fail;

		if (NodeCount >= BucketCount)
		{
			Index = (BucketCount > 0) ? (BucketCount << 1) : 1;

			if (Resize(Index))
				goto Fail;
		}

		AddNode(HashNode);
		NodeCount++;
		return 0;

	Fail:
		if (HashNode)
			free(HashNode);

		return -1;
	}

	void Remove(const char *Key)
	{
		StringHashNode *HashNode;
		StringHashNode **Next = GetReference(Key);

		if (Next)
		{
			HashNode = *Next;
			*Next = (*Next)->Next;
			free(HashNode);
			NodeCount--;
		}
	}

	int Length()
	{
		return NodeCount;
	}

	static uint32_t HashValue(const char *Key)
	{
		uint32_t Hash = 0;

		while (*Key)
			Hash = Hash * 101 + *Key++;

		return Hash;
	}

private:
	struct StringHashNode
	{
		char *Key;
		int Value;
		uint32_t Hash;
		StringHashNode *Next;
	};

	StringHashNode **Buckets = NULL;
	int BucketCount = 0, NodeCount = 0;

	StringHashNode *CreateNode(const char *Key, int Value)
	{
		StringHashNode *HashNode;
		HashNode = (StringHashNode *)malloc(sizeof(StringHashNode));
		HashNode->Key = strdup(Key);
		HashNode->Value = Value;
		HashNode->Hash = HashValue(HashNode->Key);
		return HashNode;
	}

	int GetBucketIndex(uint32_t Hash)
	{
		return Hash & (BucketCount - 1);
	}


	void AddNode(StringHashNode *HashNode)
	{
		int Index = GetBucketIndex(HashNode->Hash);
		HashNode->Next = Buckets[Index];
		Buckets[Index] = HashNode;
	}


	int Resize(int NewBucketCount)
	{
		StringHashNode *HashNodes, *HashNode, *Next;
		StringHashNode **NewBuckets;
		int Index = BucketCount;

		HashNodes = NULL;

		while (Index--)
		{
			HashNode = (Buckets)[Index];

			while (HashNode)
			{
				Next = HashNode->Next;
				HashNode->Next = HashNodes;
				HashNodes = HashNode;
				HashNode = Next;
			}
		}

		if (!Buckets || *Buckets == NULL)
			NewBuckets = (StringHashNode **)malloc(sizeof(*Buckets) * NewBucketCount);
		else
			NewBuckets = (StringHashNode **)realloc(Buckets, sizeof(*Buckets) * NewBucketCount);

		if (NewBuckets != NULL)
		{
			Buckets = NewBuckets;
			BucketCount = NewBucketCount;
		}

		if (Buckets)
		{
			memset(Buckets, 0, sizeof(*Buckets) * BucketCount);

			HashNode = HashNodes;

			while (HashNode)
			{
				Next = HashNode->Next;
				AddNode(HashNode);
				HashNode = Next;
			}
		}

		return (NewBuckets == NULL) ? -1 : 0;
	}


	StringHashNode **GetReference(const char *Key)
	{
		uint32_t Hash = HashValue(Key);
		StringHashNode **Next;

		if (BucketCount > 0)
		{
			Next = &Buckets[GetBucketIndex(Hash)];

			while (*Next)
			{
				if ((*Next)->Hash == Hash && !strcmp((*Next)->Key, Key))
					return Next;

				Next = &(*Next)->Next;
			}
		}
		return NULL;
	}
};