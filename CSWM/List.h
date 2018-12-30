#pragma once

template <typename T> struct List
{
public:
	int Length = 0;
	T *Data = NULL;

	template <typename U> int Append(U &&Item)
	{
		T *NewData = new T[Length + 1];

		if (Data)
		{
			memcpy(NewData, Data, Length * sizeof(T));
			delete[] Data;
		}
		
		Data = NewData;
		Data[Length] = Item;
		Length++;
		return 0;
	}

	T& Get(int Index)
	{
		return Data[Index];
	}

	void Clear()
	{
		if (Data)
		{
			delete[] Data;
			Data = NULL;
		}

		Length = 0;
	}

	T& operator[] (int Index)
	{
		return Data[Index];
	}
};