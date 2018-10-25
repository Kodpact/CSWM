#pragma once

#include "18X/Config.h"
#include "182/AMXX.h"

#include "..\SDK\metamod\meta_api.h"
#include "..\SDK\metamod\meta_eiface.h"

#include "..\cssdk\dlls\cbase.h"
#include "..\cssdk\dlls\func_break.h"
#include "..\cssdk\dlls\player.h"
#include "..\cssdk\dlls\decals.h"
#include "..\cssdk\common\usercmd.h"
#include "..\cssdk\common\entity_state.h"
#include "..\cssdk\dlls\regamedll_api.h"
#include "..\cssdk\dlls\regamedll_common.h"
#include "..\cssdk\dlls\regamedll_const.h"
#include "..\cssdk\dlls\regamedll_interfaces.h"
#include "..\cssdk\dlls\monsters.h"
#include "..\cssdk\dlls\weapons.h"
#include "..\cssdk\engine\maintypes.h"
#include "..\cssdk\engine\studio.h"

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
