#include "Module.h"

void HookEntityFW(const char ClassName[], int Offset, void *Function, void *&Forward)
{
	edict_t *Edict = CREATE_ENTITY();
	CALL_GAME_ENTITY(PLID, ClassName, &Edict->v);

	if (Edict->pvPrivateData == NULL)
	{
		REMOVE_ENTITY(Edict);
		return;
	}

	void **VTable = *((void ***)((char *)Edict->pvPrivateData));

	REMOVE_ENTITY(Edict);

	if (VTable == NULL)
		return;

	int **IVTable = (int **)VTable;

	Forward = (void *)IVTable[Offset];
	DWORD OldFlags;
	VirtualProtect(&IVTable[Offset], sizeof(int *), PAGE_READWRITE, &OldFlags);
	IVTable[Offset] = (int *)Function;
}

void HookEntityFWByVTable(void **VTable, int Offset, void *Function, void *&Forward)
{
	int **IVTable = (int **)VTable;
	Forward = (void *)IVTable[Offset];
	DWORD OldFlags;
	VirtualProtect(&IVTable[Offset], sizeof(int *), PAGE_READWRITE, &OldFlags);
	IVTable[Offset] = (int *)Function;
}

void ResetEntityFW(const char ClassName[], int Offset, void *Function, void *Forward)
{
	edict_t *Edict = CREATE_ENTITY();
	CALL_GAME_ENTITY(PLID, ClassName, &Edict->v);

	if (Edict->pvPrivateData == NULL)
	{
		REMOVE_ENTITY(Edict);
		return;
	}

	void **VTable = *((void ***)((char *)Edict->pvPrivateData));

	int **IVTable = (int **)VTable;

	DWORD OldFlags;
	VirtualProtect(&IVTable[Offset], sizeof(int*), PAGE_READWRITE, &OldFlags);

	IVTable[Offset] = (int *)Forward;
	VirtualFree(Function, 0, MEM_RELEASE);
}

void ResetEntityFWByVTable(void **VTable, int Offset, void *Function, void *&Forward)
{
	int **IVTable = (int **)VTable;

	DWORD OldFlags;
	VirtualProtect(&IVTable[Offset], sizeof(int*), PAGE_READWRITE, &OldFlags);

	IVTable[Offset] = (int *)Forward;
	VirtualFree(Function, 0, MEM_RELEASE);
}

void *GetEntityFW(const char ClassName[], int Offset)
{
	edict_t *Edict = CREATE_ENTITY();
	CALL_GAME_ENTITY(PLID, ClassName, &Edict->v);

	if (Edict->pvPrivateData == NULL)
	{
		REMOVE_ENTITY(Edict);
		return nullptr;
	}

	void **VTable = *((void ***)((char *)Edict->pvPrivateData));

	REMOVE_ENTITY(Edict);

	if (VTable == NULL)
		return nullptr;

	int **IVTable = (int **)VTable;

	return (void *)IVTable[Offset];
}