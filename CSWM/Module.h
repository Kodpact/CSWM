#pragma once

#ifdef AMXX_183
	#include <183/amxxmodule.h>
#else
	#include <182/amxxmodule.h>
#endif

#include <18X/Config.h>

#include <metamod\meta_api.h>
#include <metamod\meta_eiface.h>

#include <dlls\cbase.h>
#include <dlls\func_break.h>
#include <dlls\player.h>
#include <dlls\decals.h>
#include <common\usercmd.h>
#include <common\entity_state.h>
#include <dlls\regamedll_api.h>
#include <dlls\regamedll_common.h>
#include <dlls\regamedll_const.h>
#include <dlls\regamedll_interfaces.h>
#include <dlls\monsters.h>
#include <dlls\weapons.h>
#include <engine\maintypes.h>
#include <engine\studio.h>

#include <amtl\am-vector.h>

#include <TraceAttack.h>
#include <Attack2.h>

#include <CSWM.h>

#include <CStrike.h>
#include <HEFW.h>
#include <FakeCMD.h>
#include <String.h>

extern enginefuncs_t g_engfuncs;
extern globalvars_t  *gpGlobals;

extern DLL_FUNCTIONS *g_pFunctionTable;
extern DLL_FUNCTIONS *g_pFunctionTable_Post;
extern enginefuncs_t *g_pengfuncsTable;
extern enginefuncs_t *g_pengfuncsTable_Post;
extern NEW_DLL_FUNCTIONS *g_pNewFunctionsTable;
extern NEW_DLL_FUNCTIONS *g_pNewFunctionsTable_Post;

extern IReGameApi *ReGameApi;
extern const ReGameFuncs_t *ReGameFuncs;
extern IReGameHookchains *ReGameHookchains;
