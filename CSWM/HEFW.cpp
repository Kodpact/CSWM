#include "Module.h"

extern meta_globals_t *META_GLOBALS;
extern gamedll_funcs_t *META_GAMEDLLFUNCS;
extern mutil_funcs_t *META_UTILFUNCS;

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

#if defined(_WIN32)
	DWORD OldFlags;
	VirtualProtect(&IVTable[Offset], sizeof(int *), PAGE_READWRITE, &OldFlags);
#else
	void *Address = (void *)ALIGN(&IVTable[Offset]);
	mprotect(Address, sysconf(_SC_PAGESIZE), PROT_READ | PROT_WRITE);
#endif

	IVTable[Offset] = (int *)Function;
}

void HookEntityFWByVTable(void **VTable, int Offset, void *Function, void *&Forward)
{
	int **IVTable = (int **)VTable;
	Forward = (void *)IVTable[Offset];

#if defined(_WIN32)
	DWORD OldFlags;
	VirtualProtect(&IVTable[Offset], sizeof(int *), PAGE_READWRITE, &OldFlags);
#else
	void *Address = (void *)ALIGN(&IVTable[Offset]);
	mprotect(Address, sysconf(_SC_PAGESIZE), PROT_READ | PROT_WRITE);
#endif

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
		return NULL;
	}

	void **VTable = *((void ***)((char *)Edict->pvPrivateData));

	REMOVE_ENTITY(Edict);

	if (VTable == NULL)
		return NULL;

	int **IVTable = (int **)VTable;

	return (void *)IVTable[Offset];
}

