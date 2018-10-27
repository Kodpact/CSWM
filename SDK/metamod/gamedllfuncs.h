#ifndef INCLUDE_GAMEDLLFUNCS_H
#define INCLUDE_GAMEDLLFUNCS_H

typedef struct
{
	DLL_FUNCTIONS *dllapi_table;
	NEW_DLL_FUNCTIONS *newapi_table;
} gamedll_funcs_t;

#endif