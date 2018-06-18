#pragma once
#include "Module.h"
#include "PlatFormHelper.h"

IReGameApi *ReGameApi;
const ReGameFuncs_t *ReGameFuncs;
IReGameHookchains *ReGameHookchains;

#include <os/am-path.h>
#include <os/am-shared-library.h>

template <typename T>
bool GET_IFACE(const char* library, T*& var, const char* version, bool add_ext = true)
{
	char file[PLATFORM_MAX_PATH];

	if (add_ext)
		ke::path::Format(file, sizeof(file), "%s.%s", library, PLATFORM_LIB_EXT);
	else
		ke::SafeStrcpy(file, sizeof(file), library);

	auto lib = ke::SharedLib::Open(file);

	if (!lib || !lib->valid())
	{
		return false;
	}

	auto factory = reinterpret_cast<CreateInterfaceFn>(lib->lookup(CREATEINTERFACE_PROCNAME));

	if (factory)
	{
		var = reinterpret_cast<T*>(factory(version, NULL));
		return true;
	}

	var = NULL;

	return false;
}

bool ReGameDLL_API_Initialize()
{
	auto library = GET_GAME_INFO(PLID, GINFO_DLL_FULLPATH);

	if (!library || !GET_IFACE<IReGameApi>(library, ReGameApi, VRE_GAMEDLL_API_VERSION, false) || !ReGameApi)
	{
		return false;
	}

	auto majorVersion = ReGameApi->GetMajorVersion();
	auto minorVersion = ReGameApi->GetMinorVersion();

	if (majorVersion != REGAMEDLL_API_VERSION_MAJOR || minorVersion < REGAMEDLL_API_VERSION_MINOR)
		return false;

	ReGameFuncs = ReGameApi->GetFuncs();
	ReGameHookchains = ReGameApi->GetHookchains();
	return true;
}