#ifndef INCLUDE_NEWDLLFUNCTIONS_H
#define INCLUDE_NEWDLLFUNCTIONS_H

#define NEW_DLL_FUNCTIONS_VERSION 1

typedef struct
{
	// Called right before the object's memory is freed. 
	// Calls its destructor.
	void(*pfnOnFreeEntPrivateData)(edict_t *pEnt);
	void(*pfnGameShutdown)(void);
	int(*pfnShouldCollide)(edict_t *pentTouched, edict_t *pentOther);
	void(*pfnCvarValue)(const edict_t *pEnt, const char *value);
	void(*pfnCvarValue2)(const edict_t *pEnt, int requestID, const char *cvarName, const char *value);
} NEW_DLL_FUNCTIONS;
typedef int(*NEW_DLL_FUNCTIONS_FN)(NEW_DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion);

#endif