#pragma once

template <typename T> struct List
{
public:
	int Length = 0;
	T *Data = NULL;
	
	T *Allocate()
	{
		T *NewData = (T *)realloc(Data, (Length + 1) * sizeof(T) * 4);

		if (!NewData)
		{
			free(Data);
			return NULL;
		}

		return NewData;
	}

public:

	template <typename U> int Append(U &&Item)
	{
		Data = Allocate();
		
		if (!Data)
			return -1;

		Data[Length] = Item;
		Length++;
		return 0;
	}

	T& Get(int Index)
	{
		return Data[Index];
	}

	size_t Size()
	{
		return Length;
	}

	void Clear()
	{
		free(Data);
		Data = NULL;
		Length = 0;
	}

	T& operator[] (int Index)
	{
		return Data[Index];
	}
};