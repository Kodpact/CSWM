#ifndef INCLUDE_METAMOD_H
#define INCLUDE_METAMOD_H

#define META_INTERFACE_VERSION "5:13"

#define META_GLOBALS gpMetaGlobals
#define META_GAMEDLLFUNCS gpGamedllFuncs
#define META_UTILFUNCS gpMetaUtilFuncs
#define PLINFO PLUGIN_INFO
#define PLID &PLUGIN_INFO

#include "metares.h"
#include "plinfo.h"
#include "metaglobals.h"
#include "gamedllfuncs.h"
#include "dllapi.h"
#include "engineapi.h"
#include "metafunctions.h"
#include "ginfo.h"
#include "mutil.h"
#include "metadllfuncs.h"

extern plugin_info_t PLINFO;

#endif
