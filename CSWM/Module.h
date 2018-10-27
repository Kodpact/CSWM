#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <Windows.h>

#include "18X/Config.h"
#include "182/AMXX.h"

#include "cssdk.h"
#include "metamod.h"

#include "Attack2.h"
#include "CSWM.h"
#include "CStrike.h"
#include "HEFW.h"
#include "FakeCMD.h"
#include "HashMap.h"

extern enginefuncs_t g_engfuncs;
extern globalvars_t  *gpGlobals;

extern DLL_FUNCTIONS *g_pFunctionTable;
extern DLL_FUNCTIONS *g_pFunctionTable_Post;
extern enginefuncs_t *g_pengfuncsTable;
extern enginefuncs_t *g_pengfuncsTable_Post;
extern NEW_DLL_FUNCTIONS *g_pNewFunctionsTable;
extern NEW_DLL_FUNCTIONS *g_pNewFunctionsTable_Post;

extern meta_globals_t *META_GLOBALS;
extern gamedll_funcs_t *META_GAMEDLLFUNCS;
extern mutil_funcs_t *META_UTILFUNCS;
