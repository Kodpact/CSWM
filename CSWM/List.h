#pragma once

#include <Module.h>

template <typename T> static uint32_t Log2(T value);

template <> inline uint32_t Log2<uint32_t>(uint32_t Number)
{
	unsigned long RVal;
	_BitScanReverse(&RVal, Number);
	return RVal;
}

static inline BOOL IsUintPtrAddSafe(size_t A, size_t B)
{
	if (!A || !B)
		return TRUE;

	size_t Log2A = Log2(A);
	size_t Log2B = Log2(B);
	return (Log2A < sizeof(size_t) * 8) && (Log2B < sizeof(size_t) * 8);
}

static inline BOOL IsUintPtrMultiplySafe(size_t A, size_t B)
{
	if (A <= 1 || B <= 1)
		return TRUE;

	size_t Log2A = Log2(A);
	size_t Log2B = Log2(B);
	return Log2A + Log2B <= sizeof(size_t) * 8;
}

template <typename T> static inline void MoveRange(T *Destination, T *Source, size_t Length)
{
	for (size_t Index = 0; Index < Length; Index++)
	{
		new (&Destination[Index]) T(ke::Move(Source[Index]));
		Source[Index].~T();
	}
}

template <typename T> struct CList
{
private:
	size_t Length = 0;
	size_t MaxSize = 0;
	T *ValuePack = NULL;
	
	BOOL GrowInSize()
	{
		if (!IsUintPtrAddSafe(Length, 1))
		{
			abort();
			return FALSE;
		}
		
		if (Length + 1 < MaxSize)
			return TRUE;

		size_t NewMaxSize = MaxSize ? MaxSize : 8;
		
		while (Length + 1 > NewMaxSize)
		{
			if (!IsUintPtrMultiplySafe(NewMaxSize, 2))
			{
				abort();
				return FALSE;
			}

			NewMaxSize *= 2;
		}

		T* NewData = (T *)malloc(sizeof(T) * NewMaxSize);

		if (NewData == nullptr)
			return FALSE;

		MoveRange<T>(NewData, ValuePack, Length);
		free(ValuePack);
		ValuePack = NewData;
		MaxSize = NewMaxSize;
		return TRUE;
	}

public:
	void Append(T Item)
	{
		if (!GrowInSize())
			return;

		ValuePack[Length] = Item;
		Length++;
	}

	T Get(int Index)
	{
		return ValuePack[Index];
	}

	size_t Size()
	{
		return Length;
	}

	void Clear()
	{
		free(ValuePack);
		ValuePack = NULL;
		MaxSize = Length =  0;
	}

	T& operator[] (int Index)
	{
		return ValuePack[Index];
	}
};