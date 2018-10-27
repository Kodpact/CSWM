#ifndef INCLUDE_METADLLFUNCS_H
#define INCLUDE_METADLLFUNCS_H

#define MDLL_FUNC	META_GAMEDLLFUNCS->dllapi_table

#define MDLL_GameDLLInit				MDLL_FUNC->pfnGameInit
#define MDLL_Spawn						MDLL_FUNC->pfnSpawn
#define MDLL_Think						MDLL_FUNC->pfnThink
#define MDLL_Use						MDLL_FUNC->pfnUse
#define MDLL_Touch						MDLL_FUNC->pfnTouch
#define MDLL_Blocked					MDLL_FUNC->pfnBlocked
#define MDLL_KeyValue					MDLL_FUNC->pfnKeyValue
#define MDLL_Save						MDLL_FUNC->pfnSave
#define MDLL_Restore					MDLL_FUNC->pfnRestore
#define MDLL_ObjectCollsionBox			MDLL_FUNC->pfnAbsBox
#define MDLL_SaveWriteFields			MDLL_FUNC->pfnSaveWriteFields
#define MDLL_SaveReadFields				MDLL_FUNC->pfnSaveReadFields
#define MDLL_SaveGlobalState			MDLL_FUNC->pfnSaveGlobalState
#define MDLL_RestoreGlobalState			MDLL_FUNC->pfnRestoreGlobalState
#define MDLL_ResetGlobalState			MDLL_FUNC->pfnResetGlobalState
#define MDLL_ClientConnect				MDLL_FUNC->pfnClientConnect
#define MDLL_ClientDisconnect			MDLL_FUNC->pfnClientDisconnect
#define MDLL_ClientKill					MDLL_FUNC->pfnClientKill
#define MDLL_ClientPutInServer			MDLL_FUNC->pfnClientPutInServer
#define MDLL_ClientCommand				MDLL_FUNC->pfnClientCommand
#define MDLL_ClientUserInfoChanged		MDLL_FUNC->pfnClientUserInfoChanged
#define MDLL_ServerActivate				MDLL_FUNC->pfnServerActivate
#define MDLL_ServerDeactivate			MDLL_FUNC->pfnServerDeactivate
#define MDLL_PlayerPreThink				MDLL_FUNC->pfnPlayerPreThink
#define MDLL_PlayerPostThink			MDLL_FUNC->pfnPlayerPostThink
#define MDLL_StartFrame					MDLL_FUNC->pfnStartFrame
#define MDLL_ParmsNewLevel				MDLL_FUNC->pfnParmsNewLevel
#define MDLL_ParmsChangeLevel			MDLL_FUNC->pfnParmsChangeLevel
#define MDLL_GetGameDescription			MDLL_FUNC->pfnGetGameDescription
#define MDLL_PlayerCustomization		MDLL_FUNC->pfnPlayerCustomization
#define MDLL_SpectatorConnect			MDLL_FUNC->pfnSpectatorConnect
#define MDLL_SpectatorDisconnect		MDLL_FUNC->pfnSpectatorDisconnect
#define MDLL_SpectatorThink				MDLL_FUNC->pfnSpectatorThink
#define MDLL_Sys_Error					MDLL_FUNC->pfnSys_Error
#define MDLL_PM_Move					MDLL_FUNC->pfnPM_Move
#define MDLL_PM_Init					MDLL_FUNC->pfnPM_Init
#define MDLL_PM_FindTextureType			MDLL_FUNC->pfnPM_FindTextureType
#define MDLL_SetupVisibility			MDLL_FUNC->pfnSetupVisibility
#define MDLL_UpdateClientData			MDLL_FUNC->pfnUpdateClientData
#define MDLL_AddToFullPack				MDLL_FUNC->pfnAddToFullPack
#define MDLL_CreateBaseline				MDLL_FUNC->pfnCreateBaseline
#define MDLL_RegisterEncoders			MDLL_FUNC->pfnRegisterEncoders
#define MDLL_GetWeaponData				MDLL_FUNC->pfnGetWeaponData
#define MDLL_CmdStart					MDLL_FUNC->pfnCmdStart
#define MDLL_CmdEnd						MDLL_FUNC->pfnCmdEnd
#define MDLL_ConnectionlessPacket		MDLL_FUNC->pfnConnectionlessPacket
#define MDLL_GetHullBounds				MDLL_FUNC->pfnGetHullBounds
#define MDLL_CreateInstancedBaselines	MDLL_FUNC->pfnCreateInstancedBaselines
#define MDLL_InconsistentFile			MDLL_FUNC->pfnInconsistentFile
#define MDLL_AllowLagCompensation		MDLL_FUNC->pfnAllowLagCompensation

// NEW API functions:
#define MNEW_FUNC	gpGamedllFuncs->newapi_table

#define MNEW_OnFreeEntPrivateData		MNEW_FUNC->pfnOnFreeEntPrivateData
#define MNEW_GameShutdown				MNEW_FUNC->pfnGameShutdown
#define MNEW_ShouldCollide				MNEW_FUNC->pfnShouldCollide
#define MNEW_CvarValue                                  MNEW_FUNC->pfnCvarValue

#endif