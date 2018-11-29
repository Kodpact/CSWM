#include "Module.h"

#ifndef _WIN32
#define ALIGN(ar) ((intptr_t)ar & ~(sysconf(_SC_PAGESIZE)-1))
#endif

extern meta_globals_t *META_GLOBALS;
extern gamedll_funcs_t *META_GAMEDLLFUNCS;
extern mutil_funcs_t *META_UTILFUNCS;

void HookEntityFW(const char ClassName[], int Offset, void *Function, void **Forward)
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

	*Forward = (void *)IVTable[Offset];

#if defined(_WIN32)
	DWORD OldFlags;
	VirtualProtect(&IVTable[Offset], sizeof(int *), PAGE_READWRITE, &OldFlags);
#else
	void *Address = (void *)ALIGN(&IVTable[Offset]);
	mprotect(Address, sysconf(_SC_PAGESIZE), PROT_READ | PROT_WRITE);
#endif

	IVTable[Offset] = (int *)Function;
}

void HookEntityFWByVTable(void **VTable, int Offset, void *Function, void **Forward)
{
	int **IVTable = (int **)VTable;
	*Forward = (void *)IVTable[Offset];

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

	REMOVE_ENTITY(Edict);
	
#if defined(_WIN32)
	DWORD OldFlags;
	VirtualProtect(&IVTable[Offset], sizeof(int *), PAGE_READWRITE, &OldFlags);
#else
	void *Address = (void *)ALIGN(&IVTable[Offset]);
	mprotect(Address, sysconf(_SC_PAGESIZE), PROT_READ | PROT_WRITE);
#endif
	
	IVTable[Offset] = (int *)Forward;

#if defined(_WIN32)
	VirtualFree(Function, 0, MEM_RELEASE);
#else
	munmap(Function, sizeof(int));
#endif
}

void ResetEntityFWByVTable(void **VTable, int Offset, void *Function, void *&Forward)
{
	int **IVTable = (int **)VTable;

#if defined(_WIN32)
	DWORD OldFlags;
	VirtualProtect(&IVTable[Offset], sizeof(int *), PAGE_READWRITE, &OldFlags);
#else
	void *Address = (void *)ALIGN(&IVTable[Offset]);
	mprotect(Address, sysconf(_SC_PAGESIZE), PROT_READ | PROT_WRITE);
#endif

	IVTable[Offset] = (int *)Forward;

#if defined(_WIN32)
	VirtualFree(Function, 0, MEM_RELEASE);
#else
	munmap(Function, sizeof(int));
#endif
}

void *GetEntityFW(const char ClassName[], int Offset)
{
	edict_t *Edict = CREATE_ENTITY();
	CALL_GAME_ENTITY(PLID, ClassName, &Edict->v);
	void **VTable = *(void ***)Edict->pvPrivateData;
	REMOVE_ENTITY(Edict);
	
	if (VTable == NULL)
	{
		LOG_CONSOLE(PLID, "[CSWM] Virtual table is invalid! (Class = '%s', Offset = '%i')", ClassName, Offset);
		return NULL;
	}

	if (!VTable[Offset])
	{
		LOG_CONSOLE(PLID, "[CSWM] Virtual table is invalid! (Update HLDS or remove module)");
		return NULL;
	}

	return VTable[Offset];
}

