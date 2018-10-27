#ifndef INCLUDE_DLLFUNCTIONS_H
#define INCLUDE_DLLFUNCTIONS_H

typedef struct
{
	// Initialize/shutdown the game (one-time call after loading of game .dll )
	void(*pfnGameInit)			(void);
	int(*pfnSpawn)				(edict_t *pent);
	void(*pfnThink)				(edict_t *pent);
	void(*pfnUse)				(edict_t *pentUsed, edict_t *pentOther);
	void(*pfnTouch)				(edict_t *pentTouched, edict_t *pentOther);
	void(*pfnBlocked)			(edict_t *pentBlocked, edict_t *pentOther);
	void(*pfnKeyValue)			(edict_t *pentKeyvalue, KeyValueData *pkvd);
	void(*pfnSave)				(edict_t *pent, SAVERESTOREDATA *pSaveData);
	int(*pfnRestore)			(edict_t *pent, SAVERESTOREDATA *pSaveData, int globalEntity);
	void(*pfnSetAbsBox)			(edict_t *pent);

	void(*pfnSaveWriteFields)	(SAVERESTOREDATA *, const char *, void *, TYPEDESCRIPTION *, int);
	void(*pfnSaveReadFields)	(SAVERESTOREDATA *, const char *, void *, TYPEDESCRIPTION *, int);

	void(*pfnSaveGlobalState)		(SAVERESTOREDATA *);
	void(*pfnRestoreGlobalState)	(SAVERESTOREDATA *);
	void(*pfnResetGlobalState)		(void);

	qboolean(*pfnClientConnect)		(edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[128]);

	void(*pfnClientDisconnect)	(edict_t *pEntity);
	void(*pfnClientKill)		(edict_t *pEntity);
	void(*pfnClientPutInServer)	(edict_t *pEntity);
	void(*pfnClientCommand)		(edict_t *pEntity);
	void(*pfnClientUserInfoChanged)(edict_t *pEntity, char *infobuffer);

	void(*pfnServerActivate)	(edict_t *pEdictList, int edictCount, int clientMax);
	void(*pfnServerDeactivate)	(void);

	void(*pfnPlayerPreThink)	(edict_t *pEntity);
	void(*pfnPlayerPostThink)	(edict_t *pEntity);

	void(*pfnStartFrame)		(void);
	void(*pfnParmsNewLevel)		(void);
	void(*pfnParmsChangeLevel)	(void);

	// Returns string describing current .dll.  E.g., TeamFotrress 2, Half-Life
	const char     *(*pfnGetGameDescription)(void);

	// Notify dll about a player customization.
	void(*pfnPlayerCustomization) (edict_t *pEntity, customization_t *pCustom);

	// Spectator funcs
	void(*pfnSpectatorConnect)		(edict_t *pEntity);
	void(*pfnSpectatorDisconnect)	(edict_t *pEntity);
	void(*pfnSpectatorThink)		(edict_t *pEntity);

	// Notify game .dll that engine is going to shut down.  Allows mod authors to set a breakpoint.
	void(*pfnSys_Error)			(const char *error_string);

	void(*pfnPM_Move) (struct playermove_s *ppmove, qboolean server);
	void(*pfnPM_Init) (struct playermove_s *ppmove);
	char(*pfnPM_FindTextureType)(char *name);
	void(*pfnSetupVisibility)(struct edict_s *pViewEntity, struct edict_s *pClient, unsigned char **pvs, unsigned char **pas);
	void(*pfnUpdateClientData) (const struct edict_s *ent, int sendweapons, struct clientdata_s *cd);
	int(*pfnAddToFullPack)(struct entity_state_s *state, int e, edict_t *ent, edict_t *host, int hostflags, int player, unsigned char *pSet);
	void(*pfnCreateBaseline) (int player, int eindex, struct entity_state_s *baseline, struct edict_s *entity, int playermodelindex, vec3_t player_mins, vec3_t player_maxs);
	void(*pfnRegisterEncoders)	(void);
	int(*pfnGetWeaponData)		(struct edict_s *player, struct weapon_data_s *info);

	void(*pfnCmdStart)			(const edict_t *player, const struct usercmd_s *cmd, unsigned int random_seed);
	void(*pfnCmdEnd)			(const edict_t *player);

	// Return 1 if the packet is valid.  Set response_buffer_size if you want to send a response packet.  Incoming, it holds the max
	//  size of the response_buffer, so you must zero it out if you choose not to respond.
	int(*pfnConnectionlessPacket)	(const struct netadr_s *net_from_, const char *args, char *response_buffer, int *response_buffer_size);

	// Enumerates player hulls.  Returns 0 if the hull number doesn't exist, 1 otherwise
	int(*pfnGetHullBounds)	(int hullnumber, float *mins, float *maxs);

	// Create baselines for certain "unplaced" items.
	void(*pfnCreateInstancedBaselines) (void);

	// One of the pfnForceUnmodified files failed the consistency check for the specified player
	// Return 0 to allow the client to continue, 1 to force immediate disconnection ( with an optional disconnect message of up to 256 characters )
	int(*pfnInconsistentFile)(const struct edict_s *player, const char *filename, char *disconnect_message);

	// The game .dll should return 1 if lag compensation should be allowed ( could also just set
	//  the sv_unlag cvar.
	// Most games right now should return 0, until client-side weapon prediction code is written
	//  and tested for them.
	int(*pfnAllowLagCompensation)(void);
} DLL_FUNCTIONS;

typedef int(*APIFUNCTION)(DLL_FUNCTIONS *pFunctionTable, int interfaceVersion);
typedef int(*APIFUNCTION2)(DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion);

#endif