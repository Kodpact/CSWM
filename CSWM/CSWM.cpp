#include "Module.h"

//#pragma warning (disable : 4701)
//#pragma warning (disable : 4996)

typedef void V;

edict_t *SVGame_Edicts;
size_t PEV_Offset;

List<CWeapon> Weapons;
List<CAmmo> Ammos;
List<CProjectile> Projectiles;
List<CEffect> Effects;

StringHashMap AnimHashMap;

BOOL Activated = FALSE;
BOOL Initialized = FALSE;
BOOL EntityHooked = FALSE;
BOOL CanPrecache = TRUE;
BOOL ClearWeapons = FALSE;

void *FWeapon_Spawn[MAX_WEAPON_TYPES];
void *FWeapon_AddToPlayer[MAX_WEAPON_TYPES];
void *FWeapon_Deploy[MAX_WEAPON_TYPES];
void *FWeapon_PrimaryAttack[MAX_WEAPON_TYPES];
void *FWeapon_SecondaryAttack[MAX_WEAPON_TYPES];
void *FWeapon_Reload[MAX_WEAPON_TYPES];
void *FWeapon_PostFrame[MAX_WEAPON_TYPES];
void *FWeapon_Idle[MAX_WEAPON_TYPES];
void *FWeapon_Holster[MAX_WEAPON_TYPES];
void *FWeapon_GetMaxSpeed[MAX_WEAPON_TYPES];
void *FWeapon_ExtractAmmo[MAX_WEAPON_TYPES];
void *FWeapon_SendWeaponAnim[MAX_WEAPON_TYPES];
void *FWeapon_PlayEmptySound;
void *FWeapon_ResetEmptySound;
void *FPlayer_TakeDamage, *FPlayerBot_TakeDamage, *FEntity_TakeDamage;
void *FPlayer_GiveAmmo;
void *FTraceAttackEntity;
void *FTraceAttackPlayer;
void *FPlayerKilled;
void *FProjectileThink;
void *FProjectileTouch;
void *FEffectThink;
void *FGrenade_Spawn;

static const Vector ZVector = { 0.0f, 0.0f, 0.0f };
static void SetWeaponHUD(edict_t *Player, int WeaponID);
static void SetWeaponHUDCustom(edict_t *Player, CWeapon &Weapon);
static void ScreenShake(edict_t *PlayerEdict, int Amplitude, int Duration, int Frequency);
static void ClientPrint(edict_t *PlayerEdict, int Type, const char *Msg);
static void BarTime(edict_t *PlayerEdict, int Duration);
static void TraceAttackContinue(CBaseEntity *BaseEntity, entvars_t *AttackerVars, TraceResult *TResult);
static DECLFUNC_OS(int, Player_GiveAmmo, CBasePlayer *BasePlayer, int Amount, const char *Name, int Max);

const CAmmo DEFAULT_AMMOS[] =
{
	{ 0, 0, 0, "\0" },
	{ AMMO_338MAG_PRICE, AMMO_338MAG_BUY, MAX_AMMO_338MAGNUM, "338Magnum" },
	{ AMMO_762MM_PRICE, AMMO_762NATO_BUY, MAX_AMMO_762NATO, "762Nato" },
	{ AMMO_556MM_PRICE, AMMO_556NATOBOX_BUY, MAX_AMMO_556NATOBOX, "556NatoBox" },
	{ AMMO_556MM_PRICE, AMMO_556NATO_BUY, MAX_AMMO_556NATO, "556Nato" },
	{ AMMO_BUCKSHOT_PRICE, AMMO_BUCKSHOT_BUY, MAX_AMMO_BUCKSHOT, "buckshot" },
	{ AMMO_45ACP_PRICE, AMMO_45ACP_BUY, MAX_AMMO_45ACP, "45acp" },
	{ AMMO_57MM_PRICE, AMMO_57MM_BUY, MAX_AMMO_57MM, "57mm" },
	{ AMMO_50AE_PRICE, AMMO_50AE_BUY, MAX_AMMO_50AE, "50AE" },
	{ AMMO_357SIG_PRICE, AMMO_357SIG_BUY, MAX_AMMO_357SIG, "357SIG" },
	{ AMMO_9MM_PRICE, AMMO_9MM_BUY, MAX_AMMO_9MM, "9mm" },
	{ 200, AMMO_FLASHBANG, -1, "Flashbang" },
	{ 300, AMMO_HEGRENADE, -1, "HEGrenade" },
	{ 300, AMMO_SMOKEGRENADE, -1, "SmokeGrenade" },
	{ 0, AMMO_C4, -1, "C4" },
};

int MI_Ring;
int MI_SmokePuff;

const char WEAPON_CLASS[31][20] = { "", "weapon_p228", "weapon_shield", "weapon_scout", "weapon_hegrenade", "weapon_xm1014", "weapon_c4", "weapon_mac10", "weapon_aug", "weapon_smokegrenade", "weapon_elite", "weapon_fiveseven", "weapon_ump45", "weapon_sg550", "weapon_galil", "weapon_famas", "weapon_usp", "weapon_glock18", "weapon_awp", "weapon_mp5navy", "weapon_m249", "weapon_m3", "weapon_m4a1", "weapon_tmp", "weapon_g3sg1", "weapon_flashbang", "weapon_deagle", "weapon_sg552", "weapon_ak47", "weapon_knife", "weapon_p90" };
const int WEAPON_FID[] = { 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0 };
const int WEAPON_SLOT[] = { 0, 2, 2, 1, 4, 1, 5, 1, 1, 4, 2, 2, 1, 1, 1, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 4, 2, 1, 1, 3, 1 };
const int WEAPON_SLOTID[] = { 1, 0, 0, 0 };
const int WEAPON_NUMINSLOT[] = { 3, 12, 1, 2 };
const int WEAPON_WEIGHT[] = { P228_WEIGHT, XM1014_WEIGHT, AK47_WEIGHT, AWP_WEIGHT };
const int WEAPON_TYPE_ID[] = { CSW_P228, CSW_XM1014, CSW_AK47, CSW_AWP };
//const int WEAPON_CLIP[] = { 13, 7, 30, 10 };
const int WEAPONS_BIT_SUM = 1577057706;
const int SECONDARY_WEAPONS_BIT_SUM = 67308546;
const float WEAPON_MIN_RECOIL = -8.0f;
const float WEAPON_MAX_RECOIL = 8.0f;
const int GUNSHOT_DECALS[] = { 41, 42, 43, 44, 45 };
const char PROJECTILE_CLASS[] = "grenade";
const char EFFECT_CLASS[] = "env_sprite";
const char PLAYER_CLASS[] = "player";
const char SHOTGUN_RELOAD_ANIM[2][22] = { "ref_reload_shotgun", "crouch_reload_shotgun" };

void PrecacheModule(void);
void **SpecialBot_VTable = NULL;

extern cell WeaponCount;
extern cell AmmoCount;

extern CCleaveDamageInfo CleaveDamageInfo;
extern CKnockbackInfo KnockbackInfo;

cvar_t CVar_Version = { "cswm_version", MODULE_VERSION, FCVAR_ARCHIVE | FCVAR_SERVER, 0.0f, NULL };
cvar_t CVar_Log = { "cswm_log", "0", FCVAR_ARCHIVE | FCVAR_SERVER, 0.0f, NULL };
cvar_t CVar_AMap = { "cswm_animmap", "1", FCVAR_ARCHIVE | FCVAR_SERVER, 0.0f, NULL };
cvar_t CVar_SmokePuff = { "cswm_smokepuff", "0", FCVAR_ARCHIVE | FCVAR_SERVER, 0.0f, NULL };
cvar_t *CVar_LogPointer;
cvar_t *CVar_AMapPointer;
cvar_t *CVar_SmokePuffPointer;
cvar_t *CVar_CheatsPointer;

void GameDLLInit_Post(void)
{
	SET_META_RESULT(MRES_IGNORED);

	CVAR_REGISTER(&CVar_Version);
	CVAR_REGISTER(&CVar_Log);
	CVAR_REGISTER(&CVar_AMap);
	CVAR_REGISTER(&CVar_SmokePuff);

	CVar_LogPointer = CVAR_GET_POINTER("cswm_log");
	CVar_AMapPointer = CVAR_GET_POINTER("cswm_amap");
	CVar_SmokePuffPointer = CVAR_GET_POINTER("cswm_smokepuff");
	CVar_CheatsPointer = CVAR_GET_POINTER("sv_cheats");
}

static void __FC Weapon_SpawnPistol(CBasePlayerWeapon *BaseWeapon)
{
	WEAPON_FID(BaseWeapon) = WType::Pistol;
	((FN_WEAPON_SPAWN)FWeapon_Spawn[WType::Pistol])(BaseWeapon);
}

static void __FC Weapon_SpawnShotgun(CBasePlayerWeapon *BaseWeapon)
{
	WEAPON_FID(BaseWeapon) = WType::Shotgun;
	((FN_WEAPON_SPAWN)FWeapon_Spawn[WType::Shotgun])(BaseWeapon);
}

static void __FC Weapon_SpawnRifle(CBasePlayerWeapon *BaseWeapon)
{
	WEAPON_FID(BaseWeapon) = WType::Rifle;
	((FN_WEAPON_SPAWN)FWeapon_Spawn[WType::Rifle])(BaseWeapon);
}

static void __FC Weapon_SpawnSniper(CBasePlayerWeapon *BaseWeapon)
{
	WEAPON_FID(BaseWeapon) = WType::Sniper;
	((FN_WEAPON_SPAWN)FWeapon_Spawn[WType::Sniper])(BaseWeapon);
}

static DECLFUNC_OS(BOOL, Weapon_AddToPlayer, CBasePlayerWeapon *BaseWeapon, CBasePlayer *BasePlayer)
{
	CALLFUNC_GAME(FN_WEAPON_ADDTOPLAYER, FWeapon_AddToPlayer[WEAPON_FID(BaseWeapon)], BaseWeapon, BasePlayer);
	edict_t *PlayerEdict = ED_FROM_PD(BasePlayer);

	if (!CUSTOM_WEAPON(BaseWeapon))
		SetWeaponHUD(PlayerEdict, WEAPON_ID(BaseWeapon));
	else
	{
		CWeapon &Weapon = Weapons[WEAPON_KEY(BaseWeapon)];
		SetWeaponHUDCustom(PlayerEdict, Weapon);

		if (!(Weapon.Flags & WFlag::SoloClip))
			WEAPON_AMMO_TYPE(BaseWeapon) = Weapon.AmmoID;
	}
	return TRUE;
}

static BOOL __FC Weapon_Deploy(CBasePlayerWeapon *BaseWeapon)
{
	((FN_WEAPON_DEPLOY)FWeapon_Deploy[WEAPON_FID(BaseWeapon)])(BaseWeapon);

	if (!CUSTOM_WEAPON(BaseWeapon))
		return TRUE;

	CWeapon &Weapon = Weapons[WEAPON_KEY(BaseWeapon)];
	CBasePlayer *BasePlayer = GetPrivateData(CBasePlayer *, BaseWeapon, CBasePlayerItem_Player, 4);
	entvars_t *PlayerEntVars = EV_FROM_PD(BasePlayer);
	PlayerEntVars->viewmodel = Weapon.VModel;
	PlayerEntVars->weaponmodel = Weapon.PModel;
	BOOL SwitchON = (Weapon.A2I == A2_Switch && WEAPON_INA2(BaseWeapon)) ? TRUE : FALSE;

	GetPrivateData(float, BasePlayer, CBaseMonster_NextAttack, 5)
		= GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_NextSecondaryAttack, 4)
		= GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_NextPrimaryAttack, 4)
		= SwitchON ? GetAttack2Data(float, Weapon.A2V, WA2_SWITCH_ANIM_DRAW_DURATION) : Weapon.Deploy;

	SwitchON = SwitchON ? GetAttack2Data(int, Weapon.A2V, WA2_SWITCH_ANIM_DRAW) : Weapon.AnimD;
	GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_TimeWeaponIdle, 4) = Weapon.DurationList.Get(SwitchON);

	SendWeaponAnim(BaseWeapon, SwitchON);

	if (Weapon.Forwards[WForward::DeployPost])
		MF_ExecuteForward(Weapon.Forwards[WForward::DeployPost], EDI_FROM_PD(BaseWeapon));

	return TRUE;
}

#pragma warning (disable : 4701)

static void __FC Weapon_PrimaryAttack(CBasePlayerWeapon *BaseWeapon)
{
	if (!CUSTOM_WEAPON(BaseWeapon))
		return ((FN_WEAPON_PRIMARYATTACK)FWeapon_PrimaryAttack[WEAPON_FID(BaseWeapon)])(BaseWeapon);

	BOOL InAttack2;
	int Clip, FOV, A2I;
	entvars_t *PlayerEntVars;
	Vector PunchAngleOld;
	CWeapon &Weapon = Weapons[WEAPON_KEY(BaseWeapon)];
	InAttack2 = WEAPON_INA2(BaseWeapon);
	A2I = InAttack2 ? Weapon.A2I : A2_None;

	if (A2I == A2_AutoPistol && GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_NextPrimaryAttack, 4) > 0.0f)
		return;

	CBasePlayer *BasePlayer = GetPrivateData(CBasePlayer *, BaseWeapon, CBasePlayerItem_Player, 4);
	PlayerEntVars = EV_FROM_PD(BasePlayer);
	Clip = WEAPON_CLIP(BaseWeapon);

	if (GetPrivateData(int, BaseWeapon, CBasePlayerWeapon_InSpecialReload, 4) == 1 && !WEAPON_CLIP(BaseWeapon))
	{
		((FN_WEAPON_RELOAD)FWeapon_Reload[Weapon.Type])(BaseWeapon);
		return;
	}

	if (Weapon.Flags & WFlag::AutoSniper)
		FOV = GetPrivateData(int, BasePlayer, CBasePlayer_FOV, 5);

	if (!A2I && !WEAPON_CLIP(BaseWeapon))
	{
		((FN_WEAPON_PLAY_EMPTY_SOUND)FWeapon_PlayEmptySound)(BaseWeapon);
		GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_NextPrimaryAttack, 4) = 0.2f;
		return;
	}

	if ((Weapon.Forwards[WForward::PrimaryAttackPre]) && MF_ExecuteForward(Weapon.Forwards[WForward::PrimaryAttackPre], EDI_FROM_PD(BaseWeapon)) > WReturn::IGNORED)
		return;

	PunchAngleOld = PlayerEntVars->punchangle;
	((FN_WEAPON_PRIMARYATTACK)FWeapon_PrimaryAttack[WEAPON_FID(BaseWeapon)])(BaseWeapon);

	if ((Weapon.A2I == A2_Burst || Weapon.A2I == A2_MultiShot) && InAttack2)
		GetPrivateData(int, BaseWeapon, CBasePlayerWeapon_ShotsFired, 4) = FALSE;

	if (Weapon.Flags & AutoSniper)
		GetPrivateData(int, BasePlayer, CBasePlayer_FOV, 5) = FOV;

	if (Clip <= WEAPON_CLIP(BaseWeapon))
		return;

	if ((Weapon.Forwards[WForward::PrimaryAttackPrePost]) && MF_ExecuteForward(Weapon.Forwards[WForward::PrimaryAttackPrePost], EDI_FROM_PD(BaseWeapon)) > WReturn::IGNORED)
		return;

	Vector PunchAngle = PlayerEntVars->punchangle;
	VectorSub(PunchAngle, PunchAngleOld, PunchAngle);

	float Delay, Recoil;
	int Anim = 0;

	switch (A2I)
	{
		default:
		{
			Delay = Weapon.Delay;
			Recoil = Weapon.Recoil;
			break;
		}
		case A2_Switch:
		{
			Delay = GetAttack2Data(float, Weapon.A2V, WA2_SWITCH_DELAY);
			Anim = GetAttack2Data(int, Weapon.A2V, WA2_SWITCH_ANIM_SHOOT);
			Recoil = GetAttack2Data(float, Weapon.A2V, WA2_SWITCH_RECOIL);
			break;
		}
		case A2_AutoPistol:
		{
			Delay = GetAttack2Data(float, Weapon.A2V, WA2_AUTOPISTOL_DELAY);
			Anim = GetAttack2Data(int, Weapon.A2V, WA2_AUTOPISTOL_ANIM);
			Recoil = GetAttack2Data(float, Weapon.A2V, WA2_AUTOPISTOL_RECOIL);
			break;
		}
		case A2_Burst:
		{
			Delay = 0.5f;
			Recoil = Weapon.Recoil;
			break;
		}
		case A2_MultiShot:
		{
			Delay = Weapon.Delay;
			Recoil = Weapon.Recoil * 1.5f;
			break;
		}
		case A2_InstaSwitch:
		{
			Delay = GetAttack2Data(float, Weapon.A2V, WA2_INSTASWITCH_DELAY);
			Anim = GetAttack2Data(int, Weapon.A2V, WA2_INSTASWITCH_ANIM_SHOOT);
			Recoil = GetAttack2Data(float, Weapon.A2V, WA2_INSTASWITCH_RECOIL);
		}
	}

	if (!Anim)
		Anim = Weapon.AnimS[(RANDOM_LONG(0, Weapon.AnimS.Length - 1))];

	VectorMulScalar(PunchAngle, Recoil, PunchAngle);
	VectorSum(PunchAngle, PunchAngleOld, PunchAngle);
	clamp(PunchAngle.x, WEAPON_MIN_RECOIL, WEAPON_MAX_RECOIL);
	clamp(PunchAngle.y, WEAPON_MIN_RECOIL, WEAPON_MAX_RECOIL);
	clamp(PunchAngle.z, WEAPON_MIN_RECOIL, WEAPON_MAX_RECOIL);
	PlayerEntVars->punchangle = PunchAngle;

	SendWeaponAnim(BaseWeapon, Anim);
	GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_NextPrimaryAttack, 4) = Delay;
	GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_NextSecondaryAttack, 4) = 0.1f;
	GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_TimeWeaponIdle, 4) =
		(A2I == A2_Switch) ? Anim = GetAttack2Data(int, Weapon.A2V, WA2_SWITCH_ANIM_SHOOT_DURATION) : Weapon.DurationList.Get(Anim);
	EMIT_SOUND_SHORT(ENT(PlayerEntVars), CHAN_WEAPON, A2I == A2_Switch ? GetAttack2Data(const char *, Weapon.A2V, WA2_SWITCH_FSOUND) : Weapon.FireSound);

	switch (A2I)
	{
		case A2_None: break;
		case A2_Burst:
		{
			WEAPON_CURBURST(BaseWeapon)--;

			if (WEAPON_INBURST(BaseWeapon))
				break;

			GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_LastFire, 4) = gpGlobals->time;

			WEAPON_INBURST(BaseWeapon) = TRUE;
			WEAPON_CURBURST(BaseWeapon) = WEAPON_A2_OFFSET(BaseWeapon);
			break;
		}
		case A2_AutoPistol:
		{
			if (InAttack2)
				ScreenShake(ED_FROM_PD(BasePlayer), 4, 3, 7);

			break;
		}
	}

	if (Weapon.Forwards[WForward::PrimaryAttackPost])
		MF_ExecuteForward(Weapon.Forwards[WForward::PrimaryAttackPost], EDI_FROM_PD(BaseWeapon));

	if (Weapon.Flags & WFlag::AutoReload)
		((FN_WEAPON_RELOAD)FWeapon_Reload[Weapon.Type])(BaseWeapon);
}

static void __FC Weapon_SecondaryAttack(CBasePlayerWeapon *BaseWeapon)
{
	if (!CUSTOM_WEAPON(BaseWeapon))
		((FN_WEAPON_SECONDARYATTACK)FWeapon_SecondaryAttack[WEAPON_FID(BaseWeapon)])(BaseWeapon);
}

static void __FC Weapon_Reload(CBasePlayerWeapon *BaseWeapon)
{
	if (!CUSTOM_WEAPON(BaseWeapon))
		return ((FN_WEAPON_RELOAD)FWeapon_Reload[WEAPON_FID(BaseWeapon)])(BaseWeapon);

	CWeapon &Weapon = Weapons[WEAPON_KEY(BaseWeapon)];

	if (CUSTOM_WEAPON(BaseWeapon) && Weapon.Forwards[WForward::ReloadPre] && MF_ExecuteForward(Weapon.Forwards[WForward::ReloadPre], EDI_FROM_PD(BaseWeapon)) > WReturn::IGNORED)
		return;

	BOOL RifleStyle = Weapon.Type == WType::Shotgun && Weapon.AnimR >= WShotgunReloadType::RifleStyle;
	((FN_WEAPON_RELOAD)FWeapon_Reload[RifleStyle ? WType::Rifle : WEAPON_FID(BaseWeapon)])(BaseWeapon);
	CBasePlayer *BasePlayer = GetPrivateData(CBasePlayer *, BaseWeapon, CBasePlayerItem_Player, 4);

	if (!GetPrivateDataEx(int, BasePlayer, CBasePlayer_Ammo, Weapon.AmmoID, 5) || WEAPON_CLIP(BaseWeapon) == Weapon.Clip)
		return;

	BOOL SwitchON = (Weapon.A2I == A2_Switch && WEAPON_INA2(BaseWeapon));

	GetPrivateData(int, BasePlayer, CBasePlayer_FOV, 5) = CS_NO_ZOOM;
	GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_NextSecondaryAttack, 4)
		= GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_NextPrimaryAttack, 4)
		= GetPrivateData(float, BasePlayer, CBaseMonster_NextAttack, 5)
		= SwitchON ? GetAttack2Data(float, Weapon.A2V, WA2_SWITCH_ANIM_RELOAD_DURATION) : Weapon.Reload;

	SwitchON = SwitchON ? GetAttack2Data(int, Weapon.A2V, WA2_SWITCH_ANIM_RELOAD) : Weapon.AnimR;
	GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_TimeWeaponIdle, 4) = Weapon.DurationList.Get(SwitchON);
	GetPrivateData(int, BaseWeapon, CBasePlayerWeapon_InReload, 4) = TRUE;
	SendWeaponAnim(BaseWeapon, SwitchON);

	if (Weapon.Forwards[WForward::ReloadPost])
		MF_ExecuteForward(Weapon.Forwards[WForward::ReloadPost], EDI_FROM_PD(BaseWeapon));
}

int LookupSequence(void *Model, const char *AnimExtension)
{
	studiohdr_t *Studio = (studiohdr_t *)Model;

	if (!Studio)
		return 0;

	mstudioseqdesc_t *SequenceDesc = (mstudioseqdesc_t *)((byte *)Studio + Studio->seqindex);

	for (int Index = 0; Index < Studio->numseq; Index++)
	{
		if (!strcmp(SequenceDesc[Index].label, AnimExtension))
			return Index;
	}

	return ACT_INVALID;
}

int GetPlayerSequence(CBasePlayer *BasePlayer, const char *AnimExtension)
{
	int Result;

	if (CVar_AMapPointer->value)
	{
		if (!AnimHashMap.Retrieve(AnimExtension, &Result))
			Result = 0;
	}
	else
		Result = LookupSequence(GET_MODEL_PTR(ED_FROM_PD(BasePlayer)), AnimExtension);

	return Result;
}

static void __FC Weapon_ReloadShotgun(CBasePlayerWeapon *BaseWeapon)
{
	if (!CUSTOM_WEAPON(BaseWeapon))
	{
		return ((FN_WEAPON_RELOAD)FWeapon_Reload[WType::Shotgun])(BaseWeapon);
	}
	else
	{
		CWeapon &Weapon = Weapons[WEAPON_KEY(BaseWeapon)];

		if (Weapon.AnimR >= WShotgunReloadType::RifleStyle)
			return Weapon_Reload(BaseWeapon);

		CBasePlayer *BasePlayer = GetPrivateData(CBasePlayer *, BaseWeapon, CBasePlayerItem_Player, 4);

		if (GetPrivateDataEx(int, BasePlayer, CBasePlayer_Ammo, WEAPON_AMMO_TYPE(BaseWeapon), 5) <= 0)
			return;

		if (WEAPON_CLIP(BaseWeapon) >= Weapon.Clip)
			return;

		if (GetPrivateData(int, BaseWeapon, CBasePlayerWeapon_InSpecialReload, 4) == 0)
		{
			SetAnimation(ED_FROM_PD(BasePlayer),
				GetPlayerSequence(GetPrivateData(CBasePlayer *, BaseWeapon, CBasePlayerItem_Player, 4),
					SHOTGUN_RELOAD_ANIM[EV_FROM_PD(BasePlayer)->bInDuck]), ACT_RELOAD, 1.0f);

			SendWeaponAnim(BaseWeapon, XM1014_START_RELOAD);

			GetPrivateData(int, BaseWeapon, CBasePlayerWeapon_InSpecialReload, 4) = 1;
			GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_NextPrimaryAttack, 4)
				= GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_NextSecondaryAttack, 4)
				= GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_TimeWeaponIdle, 4)
				= GetPrivateData(float, BasePlayer, CBaseMonster_NextAttack, 5)
				= 0.55f;
		}
		else if (GetPrivateData(int, BaseWeapon, CBasePlayerWeapon_InSpecialReload, 5) == 1)
		{
			if (GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_TimeWeaponIdle, 4) > 0.0f)
				return;

			GetPrivateData(int, BaseWeapon, CBasePlayerWeapon_InSpecialReload, 4) = 2;

			if (!(Weapon.Flags & WFlag::ShotgunCustomReloadSound))
				EMIT_SOUND(ED_FROM_PD(BasePlayer), CHAN_ITEM, RANDOM_LONG(0, 1) ? "weapons/reload1.wav" : "weapons/reload3.wav", VOL_NORM, ATTN_NORM, 0, 85 + RANDOM_LONG(0, 31));

			SendWeaponAnim(BaseWeapon, XM1014_RELOAD);
			GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_NextReload, 4) = Weapon.Reload;
			GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_TimeWeaponIdle, 4) = Weapon.DurationList.Get(XM1014_RELOAD);
		}
		else
		{
			GetPrivateData(int, BaseWeapon, CBasePlayerWeapon_Clip, 4)++;
			GetPrivateDataEx(int, BasePlayer, CBasePlayer_Ammo, WEAPON_AMMO_TYPE(BaseWeapon), 5)--;
			GetPrivateData(int, BaseWeapon, CBasePlayerWeapon_InSpecialReload, 4) = 1;
		}
	}
}

static void Weapon_PostFrame_SecondaryAttack_Pre(CBasePlayerWeapon *BaseWeapon, int AttackIndex)
{
	BOOL InAttack2 = WEAPON_INA2_DELAY(BaseWeapon);

	switch (AttackIndex)
	{
		case A2_None: break;
		case A2_Switch:
		{
			if (InAttack2 && GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_NextPrimaryAttack, 4) <= 0.0f)
			{
				WEAPON_INA2_DELAY(BaseWeapon) = FALSE;
				WEAPON_INA2(BaseWeapon) = !WEAPON_INA2(BaseWeapon);
			}
			break;
		}
		case A2_Burst:
		{
			if (!WEAPON_INBURST(BaseWeapon))
				break;

			if (WEAPON_CURBURST(BaseWeapon))
			{
				if (!GetPrivateData(int, BaseWeapon, CBasePlayerWeapon_InReload, 4) && WEAPON_CLIP(BaseWeapon) && gpGlobals->time - 0.025f > GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_LastFire, 4))
					return Weapon_PrimaryAttack(BaseWeapon);
			}

			if ((1 << WEAPON_ID(BaseWeapon)) & SECONDARY_WEAPONS_BIT_SUM)
				GetPrivateData(int, BaseWeapon, CBasePlayerWeapon_ShotsFired, 4) = TRUE;

			WEAPON_CURBURST(BaseWeapon) = 0;
			WEAPON_INBURST(BaseWeapon) = FALSE;
			break;
		}
		case A2_AutoPistol:
		{
			if (WEAPON_INA2(BaseWeapon))
				((FN_WEAPON_PRIMARYATTACK)FWeapon_PrimaryAttack[Weapons[WEAPON_KEY(BaseWeapon)].Type])(BaseWeapon);

			break;
		}
		case A2_KnifeAttack:
		{
			if (InAttack2 && GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_NextSecondaryAttack, 4) <= 0.0f)
				Attack2_KnifeAttackPerform(BaseWeapon);

			break;
		}
	}
}

static void Weapon_PostFrame_SecondaryAttack_Post(CBasePlayerWeapon *BaseWeapon)
{
	CBasePlayer *BasePlayer = GetPrivateData(CBasePlayer *, BaseWeapon, CBasePlayerItem_Player, 4);
	entvars_t *PlayerVars = EV_FROM_PD(BasePlayer);

	if (PlayerVars->button & IN_ATTACK2)
	{
		if (GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_NextSecondaryAttack, 4) > 0.0f)
			return;

		PlayerVars->button &= ~IN_ATTACK2;
		CWeapon &Weapon = Weapons[WEAPON_KEY(BaseWeapon)];

		if ((Weapon.Forwards[WForward::SecondaryAttackPre]) && MF_ExecuteForward(Weapon.Forwards[WForward::SecondaryAttackPre], EDI_FROM_PD(BaseWeapon)) > WReturn::IGNORED)
			return;

		switch (Weapon.A2I)
		{
			case A2_Zoom: Attack2_Zoom(BasePlayer, BaseWeapon, Weapon); GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_NextSecondaryAttack, 4) = 0.3f; break;
			case A2_Switch: Attack2_Switch(BasePlayer, BaseWeapon, Weapon); break;
			case A2_Burst: Attack2_Burst(BasePlayer, BaseWeapon, Weapon); break;
			case A2_AutoPistol: WEAPON_INA2(BaseWeapon) = TRUE; GetPrivateData(int, BaseWeapon, CBasePlayerWeapon_ShotsFired, 4) = FALSE; break;
			case A2_MultiShot: Attack2_MultiShot(BaseWeapon, Weapon); break;
			case A2_KnifeAttack: Attack2_KnifeAttack(BasePlayer, BaseWeapon, Weapon); break;
			case A2_InstaSwitch: Attack2_InstaSwitch(ED_FROM_PD(BasePlayer), BaseWeapon, Weapon); break;
			case A2_ZoomCustom: Attack2_ZoomCustom(BasePlayer, BaseWeapon, Weapon); GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_NextSecondaryAttack, 4) = 0.3f;
		}

		if (Weapon.Forwards[WForward::SecondaryAttackPost])
			MF_ExecuteForward(Weapon.Forwards[WForward::SecondaryAttackPost], EDI_FROM_PD(BaseWeapon));
	}
	else if (WEAPON_A2(BaseWeapon) == A2_AutoPistol && WEAPON_INA2(BaseWeapon))
		WEAPON_INA2(BaseWeapon) = FALSE;
}

static void CheckWeaponAttack2(CBasePlayerWeapon *BaseWeapon)
{
	if (WEAPON_A2(BaseWeapon))
	{
		Weapon_PostFrame_SecondaryAttack_Pre(BaseWeapon, WEAPON_A2(BaseWeapon));
		Weapon_PostFrame_SecondaryAttack_Post(BaseWeapon);
	}
}

static void __FC Weapon_PostFrame(CBasePlayerWeapon *BaseWeapon)
{
	if (CUSTOM_WEAPON(BaseWeapon))
	{
		if (GetPrivateData(int, BaseWeapon, CBasePlayerWeapon_InReload, 4) && GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_NextPrimaryAttack, 4) <= 0.0f)
		{
			int &Ammo = GetPrivateDataEx(int, GetPrivateData(CBasePlayer *, BaseWeapon, CBasePlayerItem_Player, 4), CBasePlayer_Ammo, WEAPON_AMMO_TYPE(BaseWeapon), 5);
			int MaxClip = Weapons[WEAPON_KEY(BaseWeapon)].Clip;
			int Add = min(MaxClip - GetPrivateData(int, BaseWeapon, CBasePlayerWeapon_Clip, 4), Ammo);
			GetPrivateData(int, BaseWeapon, CBasePlayerWeapon_Clip, 4) += Add;
			Ammo -= Add;
			GetPrivateData(int, BaseWeapon, CBasePlayerWeapon_InReload, 4) = FALSE;
		}

		CheckWeaponAttack2(BaseWeapon);
	}

	((FN_WEAPON_POSTFRAME)FWeapon_PostFrame[WEAPON_FID(BaseWeapon)])(BaseWeapon);
}

static void __FC Weapon_Idle(CBasePlayerWeapon *BaseWeapon)
{
	if (CUSTOM_WEAPON(BaseWeapon))
	{
		CWeapon &Weapon = Weapons[WEAPON_KEY(BaseWeapon)];

		if (GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_TimeWeaponIdle, 4) <= 0.0f)
		{
			int Animation;

			if ((Weapon.A2I == A2_Switch && WEAPON_INA2(BaseWeapon)))
				Animation = GetAttack2Data(int, Weapon.A2V, WA2_SWITCH_ANIM_IDLE);
			else if (Weapon.Flags & WFlag::CustomIdleAnim)
				Animation = Weapon.AnimI;
			else
				Animation = 0;

			SendWeaponAnim(BaseWeapon, Animation);
			GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_TimeWeaponIdle, 4) = Weapon.DurationList[Animation];
		}
	}
	else ((FN_WEAPON_IDLE)FWeapon_Idle[Rifle])(BaseWeapon);
}

static void __FC Weapon_IdleShotgun(CBasePlayerWeapon *BaseWeapon)
{
	if (CUSTOM_WEAPON(BaseWeapon))
	{
		CWeapon &Weapon = Weapons[WEAPON_KEY(BaseWeapon)];

		if (Weapon.AnimR)
			return Weapon_Idle(BaseWeapon);

		((FN_WEAPON_RESET_EMPTY_SOUND)FWeapon_ResetEmptySound)(BaseWeapon);
		int &Ammo = GetPrivateDataEx(int, GetPrivateData(CBasePlayer *, BaseWeapon, CBasePlayerItem_Player, 4), CBasePlayer_Ammo, WEAPON_AMMO_TYPE(BaseWeapon), 5);
		float &PumpTime = GetPrivateData(float, BaseWeapon, CXM1014_PumpTime, 4);

		if (PumpTime && PumpTime < 0.0f)
			PumpTime = 0;

		if (GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_TimeWeaponIdle, 4) < 0.0f)
		{
			if (WEAPON_CLIP(BaseWeapon) == 0 && GetPrivateData(int, BaseWeapon, CBasePlayerWeapon_InSpecialReload, 4) == 0 && Ammo)
			{
				((FN_WEAPON_RELOAD)FWeapon_Reload[Weapon.Type])(BaseWeapon);
			}
			else if (GetPrivateData(int, BaseWeapon, CBasePlayerWeapon_InSpecialReload, 4) != 0)
			{
				if (WEAPON_CLIP(BaseWeapon) != Weapon.Clip && Ammo)
				{
					((FN_WEAPON_RELOAD)FWeapon_Reload[Weapon.Type])(BaseWeapon);
				}
				else
				{
					SendWeaponAnim(BaseWeapon, XM1014_PUMP);
					GetPrivateData(int, BaseWeapon, CBasePlayerWeapon_InSpecialReload, 4) = 0;
					GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_TimeWeaponIdle, 4) = 1.5f;
				}
			}
			else
			{
				SendWeaponAnim(BaseWeapon, XM1014_IDLE);
			}
		}
	}
	else ((FN_WEAPON_IDLE)FWeapon_Idle[WType::Shotgun])(BaseWeapon);
}

static DECLFUNC_OS(void, Weapon_Holster, CBasePlayerWeapon *BaseWeapon, int SkipLocal)
{
	CALLFUNC_GAME(FN_WEAPON_HOLSTER, FWeapon_Holster[WEAPON_FID(BaseWeapon)], BaseWeapon, SkipLocal);

	if (!CUSTOM_WEAPON(BaseWeapon))
		return;

	CWeapon &Weapon = Weapons[WEAPON_KEY(BaseWeapon)];

	if (Weapon.A2I == A2_KnifeAttack && WEAPON_INA2_DELAY(BaseWeapon) && !(Weapon.Flags & WFlag::KnifeAttack_NoSound))
		STOP_SOUND(ED_FROM_PD(BaseWeapon), CHAN_VOICE, GetAttack2Data(const char *, Weapon.A2V, WA2_KNIFEATTACK_SOUND));

	if (Weapon.Flags & SwitchMode_BarTime && WEAPON_INA2(BaseWeapon))
		BarTime(ED_FROM_PD(GetPrivateData(CBasePlayer *, BaseWeapon, CBasePlayerItem_Player, 4)), 0);

	WEAPON_INA2_DELAY(BaseWeapon) = FALSE;

	if (Weapon.Type == WType::Shotgun && Weapon.AnimR < WShotgunReloadType::RifleStyle)
		GetPrivateData(int, BaseWeapon, CBasePlayerWeapon_InSpecialReload, 4) = 0;

	if ((Weapon.Forwards[WForward::HolsterPost]))
		MF_ExecuteForward(Weapon.Forwards[WForward::HolsterPost], EDI_FROM_PD(BaseWeapon));
}

static float __FC Weapon_GetMaxSpeed(CBasePlayerWeapon *BaseWeapon)
{
	if (!CUSTOM_WEAPON(BaseWeapon))
		return ((FN_WEAPON_GET_MAXSPEED)FWeapon_GetMaxSpeed[WEAPON_FID(BaseWeapon)])(BaseWeapon);

	float Speed = Weapons[WEAPON_KEY(BaseWeapon)].Speed;

	if (GetPrivateData(int, GetPrivateData(CBasePlayer *, BaseWeapon, CBasePlayerItem_Player, 4), CBasePlayer_FOV, 5) == DEFAULT_FOV)
		return Speed;

	return Speed - 40.0f;
}

static DECLFUNC_OS(int, Weapon_ExtractAmmo, CBasePlayerWeapon *BaseWeapon, CBasePlayerWeapon *Original)
{
	if (!GetPrivateData(int, BaseWeapon, CBasePlayerWeapon_DefaultAmmo, 4))
		return 0;

	if (CUSTOM_WEAPON(BaseWeapon))
	{
		int AmmoID = WEAPON_AMMO_TYPE(BaseWeapon);
		return CALLFUNC_OS(Player_GiveAmmo, GetPrivateData(CBasePlayer *, BaseWeapon, CBasePlayerItem_Player, 4), Ammos[AmmoID].Amount, Ammos[AmmoID].Name, -1);
	}
	else
		return CALLFUNC_GAME(FN_WEAPON_EXTRACT_AMMO, FWeapon_ExtractAmmo[WEAPON_FID(BaseWeapon)], BaseWeapon, Original);
}

static DECLFUNC_OS(void, Weapon_SendWeaponAnim, CBasePlayerWeapon *BaseWeapon, int Anim, int SkipLocal)
{
	if (!CUSTOM_WEAPON(BaseWeapon) || !Weapons[WEAPON_KEY(BaseWeapon)].AnimR)
		CALLFUNC_GAME(FN_WEAPON_SEND_WEAPON_ANIM, FWeapon_SendWeaponAnim[WEAPON_FID(BaseWeapon)], BaseWeapon, Anim, SkipLocal);
}

void SendWeaponAnim(CBasePlayerWeapon *BaseWeapon, int Anim)
{
	entvars_t *PlayerEntVars = EV_FROM_PD(GetPrivateData(CBasePlayer *, BaseWeapon, CBasePlayerItem_Player, 4));
	PlayerEntVars->weaponanim = Anim;
	MESSAGE_BEGIN(MSG_ONE, SVC_WEAPONANIM, NULL, ENT(PlayerEntVars));
	WRITE_BYTE(Anim);
	WRITE_BYTE(0);
	MESSAGE_END();
}

static DECLFUNC_OS(BOOL, Player_TakeDamage, CBasePlayer *BasePlayer, entvars_t *Inflictor, entvars_t *Attacker, float Damage, int DamageBits)
{
	BOOL Result;
	edict_t *PlayerEdict = ENT(Attacker);
	CBasePlayerItem *BaseItem = NULL;
	CWeapon *Weapon = NULL;

	if (!InvalidEntity(PlayerEdict) && Attacker->flags & FL_CLIENT)
		BaseItem = ((DamageBits == DMG_EXPLOSION) && Inflictor->euser1) ?
		(CBasePlayerItem *)Inflictor->euser1->pvPrivateData : GetPrivateData(CBasePlayerItem *, PlayerEdict->pvPrivateData, CBasePlayer_ActiveItem, 5);

	if (BaseItem && CUSTOM_WEAPON(BaseItem) && ((DamageBits & DMG_BULLET) | (DamageBits & DMG_EXPLOSION)))
	{
		Weapon = &Weapons[WEAPON_KEY(BaseItem)];

		if (DamageBits == DMG_BULLET)
			Damage *= (Weapon->A2I == A2_Switch && WEAPON_INA2(BaseItem)) ? GetAttack2Data(float, Weapon->A2V, WA2_SWITCH_DAMAGE) : Weapon->Damage;
	}

	/* Use Bot TakeDamage To Cause Hurt Voices & Other!? */
	Result = CALLFUNC_GAME(FN_TAKEDAMAGE, (CBaseEntity *)(EV_FROM_PD(BasePlayer)->flags & FL_FAKECLIENT ? FPlayerBot_TakeDamage : FPlayer_TakeDamage),
		(CBaseEntity *)BasePlayer, Inflictor, Attacker, Damage, DamageBits);

	if (Weapon)
	{
		if (Weapon->Forwards[WForward::DamagePost])
			MF_ExecuteForward(Weapon->Forwards[WForward::DamagePost], EDI_FROM_PD(BaseItem), Damage, EDI_FROM_PD(BasePlayer));
	}

	return Result;
}

static int GetAmmoByName(const char *Name)
{
	for (int Index = 1; Index < AmmoCount; Index++)
	{
		if (!stricmp(Name, Ammos[Index].Name))
			return Index;
	}

	return -1;
}

static BOOL CanHaveAmmo(CBasePlayer *BasePlayer, int AmmoID)
{
	if (Ammos[AmmoID].Max < 0 || GetPrivateDataEx(int, BasePlayer, CBasePlayer_Ammo, AmmoID, 5) < Ammos[AmmoID].Max)
		return TRUE;

	return FALSE;
}

static void TabulateAmmo(CBasePlayer *BasePlayer)
{
	for (int Index = 0; Index < 20; Index += 2)
		GetPrivateData(int, BasePlayer, CBaseEntity_AmmoBuckshot + Index, 5) = INT_MAX;
}

int Player_GiveAmmoByID(CBasePlayer *BasePlayer, int AmmoID, int Amount)
{
	if (AmmoID < 0)
		return -1;

	if (!CanHaveAmmo(BasePlayer, AmmoID))
		return -1;

	int Add = Ammos[AmmoID].Max == -1 ? 1 : min(Amount, Ammos[AmmoID].Max - GetPrivateDataEx(int, BasePlayer, CBasePlayer_Ammo, AmmoID, 5));

	if (Add < 1)
		return -1;

	GetPrivateDataEx(int, BasePlayer, CBasePlayer_Ammo, AmmoID, 5) += Add;
	MESSAGE_BEGIN(MSG_ONE, MESSAGE_AMMOPICKUP, NULL, ED_FROM_PD(BasePlayer));
	WRITE_BYTE(AmmoID);
	WRITE_BYTE(Add);
	MESSAGE_END();
	EMIT_SOUND_SHORT(ED_FROM_PD(BasePlayer), CHAN_ITEM, "items/9mmclip1.wav");
	TabulateAmmo(BasePlayer);
	return AmmoID;
}

static DECLFUNC_OS(int, Player_GiveAmmo, CBasePlayer *BasePlayer, int Amount, const char *Name, int Max)
{
	if (EV_FROM_PD(BasePlayer)->flags & FL_SPECTATOR)
		return -1;

	if (!Name)
		return -1;

	int AmmoID = GetAmmoByName(Name);

	return Player_GiveAmmoByID(BasePlayer, AmmoID, Amount);
}

static DECLFUNC_OS(void, TraceAttackPlayer, CBaseEntity *BaseEntity, entvars_t *AttackerVars, float Damage, Vector Direction, TraceResult *TResult, int DamageBits)
{
	CALLFUNC_GAME(FN_TRACEATTACK, FTraceAttackPlayer, BaseEntity, AttackerVars, Damage, Direction, TResult, DamageBits);

	if (DamageBits & DMG_BULLET)
		TraceAttackContinue(BaseEntity, AttackerVars, TResult);
}

static DECLFUNC_OS(void, TraceAttack, CBaseEntity *BaseEntity, entvars_t *AttackerVars, float Damage, Vector Direction, TraceResult *TResult, int DamageBits)
{
	CALLFUNC_GAME(FN_TRACEATTACK, FTraceAttackEntity, BaseEntity, AttackerVars, Damage, Direction, TResult, DamageBits);

	if (DamageBits & DMG_BULLET)
		TraceAttackContinue(BaseEntity, AttackerVars, TResult);
}

static void TraceAttackContinue(CBaseEntity *BaseEntity, entvars_t *AttackerVars, TraceResult *TResult)
{
	if (InvalidEntity(ENT(AttackerVars)))
		return;

	CBaseEntity *BaseAttacker = (CBaseEntity *)ENT(AttackerVars)->pvPrivateData;
	entvars_t *PlayerVars = EV_FROM_PD(BaseEntity);

	if (!(AttackerVars->flags & FL_CLIENT))
		return;

	CBasePlayerWeapon *BaseWeapon = GetPrivateData(CBasePlayerWeapon *, BaseAttacker, CBasePlayer_ActiveItem, 5);

	if (!BaseWeapon || !CUSTOM_WEAPON(BaseWeapon))
		return;

	if (!(WEAPON_FLAGS(BaseWeapon) & WFlag::NoDecal))
	{
		if (!(PlayerVars->flags & FL_CLIENT))
		{
			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY, NULL, NULL);
			WRITE_BYTE(TE_GUNSHOTDECAL);
			WRITE_COORD(TResult->vecEndPos.x);
			WRITE_COORD(TResult->vecEndPos.y);
			WRITE_COORD(TResult->vecEndPos.z);
			WRITE_SHORT(NUM_FOR_EDICT(TResult->pHit));
			WRITE_BYTE(GUNSHOT_DECALS[RANDOM_LONG(0, 4)]);
			MESSAGE_END();

			MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, TResult->vecEndPos, NULL);
			WRITE_BYTE(TE_STREAK_SPLASH);
			WRITE_COORD(TResult->vecEndPos.x);
			WRITE_COORD(TResult->vecEndPos.y);
			WRITE_COORD(TResult->vecEndPos.z);
			WRITE_COORD(TResult->vecPlaneNormal.x);
			WRITE_COORD(TResult->vecPlaneNormal.y);
			WRITE_COORD(TResult->vecPlaneNormal.z);
			WRITE_BYTE(4);
			WRITE_SHORT(RANDOM_LONG(25, 30));
			WRITE_SHORT(20);
			WRITE_SHORT(RANDOM_LONG(77, 85));
			MESSAGE_END();
		}
	}

	if (CVar_SmokePuffPointer->value && !(WEAPON_FLAGS(BaseWeapon) & WFlag::NoSmoke) && MI_SmokePuff)
	{
		MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, TResult->vecEndPos, NULL);
		WRITE_BYTE(TE_EXPLOSION);
		WRITE_COORD(TResult->vecEndPos.x + (3.0f * TResult->vecPlaneNormal.x));
		WRITE_COORD(TResult->vecEndPos.y + (3.0f * TResult->vecPlaneNormal.y));
		WRITE_COORD(TResult->vecEndPos.z - 10.0f + (4.0f * TResult->vecPlaneNormal.z));
		WRITE_SHORT(MI_SmokePuff);
		WRITE_BYTE(2);
		WRITE_BYTE(40);
		WRITE_BYTE(TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOSOUND | TE_EXPLFLAG_NOPARTICLES);
		MESSAGE_END();
	}
}

static void __FC ProjectileThink(CBaseEntity *BaseEntity)
{
	if (PROJECTILE_TYPE(BaseEntity) != PLIMITED_TIME)
		((FN_THINK)FProjectileThink)(BaseEntity);
	else
		MF_ExecuteForward(PROJECTILE_FORWARD(BaseEntity), EDI_FROM_PD(BaseEntity));
}

static DECLFUNC_OS(void, ProjectileTouch, CBaseEntity *BaseEntity, CBaseEntity *Touched)
{
	if (PROJECTILE_TYPE(BaseEntity) != PLIMITED_TOUCH)
		CALLFUNC_GAME(FN_TOUCH, FProjectileTouch, BaseEntity, Touched);
	else
	{
		if (EV_FROM_PD(BaseEntity)->owner != ENT(EV_FROM_PD(Touched)))
			MF_ExecuteForward(PROJECTILE_FORWARD(BaseEntity), EDI_FROM_PD(BaseEntity));
	}
}

static void __FC EffectThink(CBaseEntity *BaseEntity)
{
	if (EFFECT_TYPE(BaseEntity) != ELIMITED_TIME)
		((FN_THINK)FEffectThink)(BaseEntity);
	else
	{
		entvars_t *EntityVars = EV_FROM_PD(BaseEntity);
		float NextFrame = EntityVars->frame + EntityVars->framerate * (gpGlobals->time - EFFECT_LAST_TIME(EntityVars));
		EntityVars->frame = fmod(NextFrame, EFFECT_MAX_FRAMES(EntityVars));

		if (EFFECT_EXPIRE_TIME(BaseEntity) && gpGlobals->time >= EFFECT_EXPIRE_TIME(BaseEntity))
			MF_ExecuteForward(PROJECTILE_FORWARD(BaseEntity), NUM_FOR_EDICT(ENT(EntityVars)));
		else if (NextFrame >= EFFECT_MAX_FRAMES(EntityVars))
			MF_ExecuteForward(PROJECTILE_FORWARD(BaseEntity), NUM_FOR_EDICT(ENT(EntityVars)));

		EntityVars->nextthink = gpGlobals->time + 0.01f;
		EFFECT_LAST_TIME(EntityVars) = gpGlobals->time;
	}
}

void GiveWeaponByName(edict_t *PlayerEdict, const char *Name)
{
	if (!Name[0] || IS_USER_DEAD(PlayerEdict))
		return;

	BOOL NoError = FALSE;
	int Index;

	for (Index = 0; Index < WeaponCount; Index++)
	{
		if (!strcmp(Name, Weapons[Index].Model))
		{
			NoError = TRUE;
			break;
		}
	}

	if (!NoError)
	{
		CLIENT_PRINTF(PlayerEdict, print_console, "Invalid weapon name.\n");
		return;
	}

	GiveWeapon(PlayerEdict, Index);
}

void GiveWeapon(edict_t *PlayerEdict, int Index)
{
	if (IS_USER_DEAD(PlayerEdict))
		return;

	CWeapon &Weapon = Weapons[Index];
	edict_t *WeaponEdict = CREATE_NAMED_ENTITY(MAKE_STRING(WEAPON_CLASS[WEAPON_TYPE_ID[Weapon.Type]]));
	WeaponEdict->v.spawnflags |= SF_NORESPAWN;
	CBasePlayer *BasePlayer = (CBasePlayer *)(PlayerEdict->pvPrivateData);
	CBasePlayerWeapon *BaseWeapon = (CBasePlayerWeapon *)(WeaponEdict->pvPrivateData);
	CBasePlayerItem *BaseItem = GetPrivateDataEx(CBasePlayerItem *, BasePlayer, CBasePlayer_PlayerItems, WEAPON_SLOT[WEAPON_TYPE_ID[Weapon.Type]], 5);
	CUSTOM_WEAPON(BaseWeapon) = TRUE;
	WEAPON_AMMO_TYPE(BaseWeapon) = Weapon.AmmoID;

	if (Weapon.Flags & WFlag::SoloClip)
		GetPrivateDataEx(int, BasePlayer, CBasePlayer_Ammo, Weapon.AmmoID, 5) = Ammos[Weapon.AmmoID].Max;

	if (BaseItem)
		UTIL_FakeClientCommand(PlayerEdict, "drop", WEAPON_CLASS[WEAPON_ID(BaseItem)]);

	WEAPON_KEY(BaseWeapon) = WEAPON_KEY_EX(WeaponEdict) = Index;
	WEAPON_CLIP(BaseWeapon) = (Weapon.Flags & WFlag::SoloClip) ? -1 : Weapon.Clip;
	WEAPON_FLAGS(BaseWeapon) = Weapon.Flags;
	((FN_WEAPON_RESET_EMPTY_SOUND)FWeapon_ResetEmptySound)(BaseWeapon);

	if (Weapon.A2I)
	{
		switch (WEAPON_A2(BaseWeapon) = Weapon.A2I)
		{
			case A2_Burst: WEAPON_A2_OFFSET(BaseWeapon) = GetAttack2Data(int, Weapon.A2V, WA2_BURST_VALUE); break;
			case A2_MultiShot: WEAPON_A2_OFFSET(BaseWeapon) = GetAttack2Data(int, Weapon.A2V, WA2_MULTISHOT_VALUE);
		}
	}

	MDLL_Spawn(WeaponEdict);
	GetPrivateData(int, BaseWeapon, CBasePlayerWeapon_DefaultAmmo, 4) = 0;
	MDLL_Touch(WeaponEdict, PlayerEdict);

	if (Weapon.Forwards[WForward::SpawnPost])
		MF_ExecuteForward(Weapon.Forwards[WForward::SpawnPost], NUM_FOR_EDICT(WeaponEdict));
}

void PlaybackEvent(int Flags, const edict_t *Invoker, unsigned short EI, float Delay, float *Origin, float *Angles, float F1, float F2, int I1, int I2, int B1, int B2)
{
	SET_META_RESULT(MRES_IGNORED);

	if (!Invoker)
		return;

	CBasePlayerWeapon *BaseWeapon = GetPrivateData(CBasePlayerWeapon *, Invoker->pvPrivateData, CBasePlayer_ActiveItem, 5);

	if (!BaseWeapon || !CUSTOM_WEAPON(BaseWeapon))
		return;

	SET_META_RESULT(MRES_SUPERCEDE);
	PLAYBACK_EVENT_FULL(Flags | FEV_HOSTONLY, Invoker, EI, Delay, Origin, Angles, F1, F2, I1, I2, B1, B2);
}

void SetModel(edict_t *Edict, const char *Model)
{
	const char *ClassName = STRING(Edict->v.classname);

	if (ClassName[0] != 'w' || ClassName[6] != 'b')
		RETURN_META(MRES_IGNORED);

	CWeaponBox *WeaponBox = (CWeaponBox *)Edict->pvPrivateData;
	CBasePlayerItem *BaseItem = GetPrivateDataEx(CBasePlayerItem *, WeaponBox, CWeaponBox_PlayerItems, 1, 4);

	if (!BaseItem)
	{
		BaseItem = GetPrivateDataEx(CBasePlayerItem *, WeaponBox, CWeaponBox_PlayerItems, 2, 4);

		if (!BaseItem)
			RETURN_META(MRES_IGNORED);
	}

	if (InvalidEntity(ED_FROM_PD(BaseItem)) || !CUSTOM_WEAPON(BaseItem))
		RETURN_META(MRES_IGNORED);

	CWeapon &Weapon = Weapons[WEAPON_KEY(BaseItem)];
	SET_MODEL(Edict, Weapon.WModel);
	Edict->v.body = Weapon.WBody;

	if (Weapon.Forwards[WForward::DropPost])
		MF_ExecuteForward(Weapon.Forwards[WForward::DropPost], EDI_FROM_PD(BaseItem), NUM_FOR_EDICT(Edict));

	RETURN_META(MRES_SUPERCEDE);
}

void UpdateClientData_Post(const edict_s *PlayerEdict, int SendWeapons, clientdata_s *CD)
{
	CBasePlayerItem *BaseWeapon = GetPrivateData(CBasePlayerItem *, PlayerEdict->pvPrivateData, CBasePlayer_ActiveItem, 5);

	if (BaseWeapon && CUSTOM_WEAPON(BaseWeapon))
		CD->m_flNextAttack = gpGlobals->time + 0.001f;

	RETURN_META(MRES_IGNORED);
}

void BuyAmmo(edict_t *PlayerEdict, int Slot)
{
	CBasePlayer *BasePlayer = (CBasePlayer *)PlayerEdict->pvPrivateData;
	CBasePlayerItem *BaseItem = GetPrivateDataEx(CBasePlayerItem *, BasePlayer, CBasePlayer_PlayerItems, Slot, 5);

	if (!BaseItem)
		return;

	int &PlayerMoney = GetPrivateData(int, BasePlayer, CBasePlayer_Account, 5);
	int AmmoID = WEAPON_AMMO_TYPE(BaseItem);
	CUnifiedSignals &Signals = GetPrivateData(CUnifiedSignals, BasePlayer, CBasePlayer_Signals, 5);

	if (!(Signals.GetState() & SIGNAL_BUY))
		return;

	if (CUSTOM_WEAPON(BaseItem) && WEAPON_FLAGS(BaseItem) & WFlag::SoloClip)
		return;

	CAmmo *Ammo = &Ammos[AmmoID];

	if (GetPrivateDataEx(int, BasePlayer, CBasePlayer_Ammo, AmmoID, 5) >= Ammo->Max)
		return;

	if (Ammo->Cost > 0 && PlayerMoney < Ammo->Cost)
	{
		ClientPrint(PlayerEdict, HUD_PRINTCENTER, "#Not_Enough_Money");
		goto Flash;
	}

	Player_GiveAmmoByID(BasePlayer, AmmoID, Ammo->Amount);
	MESSAGE_BEGIN(MSG_ONE, MESSAGE_MONEY, NULL, PlayerEdict);
	WRITE_LONG(PlayerMoney -= Ammo->Cost);
	WRITE_BYTE(TRUE);
	MESSAGE_END();

Flash:
	MESSAGE_BEGIN(MSG_ONE, MESSAGE_BLINKACCT, NULL, PlayerEdict);
	WRITE_BYTE(2);
	MESSAGE_END();
}

void ClientCommand(edict_t *PlayerEdict)
{
	if (IS_USER_DEAD(PlayerEdict))
		RETURN_META(MRES_IGNORED);

	SET_META_RESULT(MRES_SUPERCEDE);

	const char *Command = CMD_ARGV(0);

	if (!stricmp(Command, "give"))
	{
		if (CVar_CheatsPointer->value)
			return GiveWeaponByName(PlayerEdict, CMD_ARGV(1));
		else
			return ClientPrint(PlayerEdict, print_console, "Give command is disabled until cheats are enabled.\n");
	}
	else if (!strncmp(Command, "weapon_", 7))
	{
		for (int Index = 0; Index < WeaponCount; Index++)
		{
			CWeapon &Weapon = Weapons[Index];

			if (!strcmp(&Command[7], Weapon.Model))
				return UTIL_FakeClientCommand(PlayerEdict, WEAPON_CLASS[WEAPON_TYPE_ID[Weapon.Type]]);
		}
	}
	else if (!stricmp(Command, "buyammo1"))
		return BuyAmmo(PlayerEdict, PRIMARY_WEAPON_SLOT);
	else if (!stricmp(Command, "buyammo2"))
		return BuyAmmo(PlayerEdict, PISTOL_SLOT);

	SET_META_RESULT(MRES_IGNORED);
}

static void SetWeaponHUD(edict_t *PlayerEdict, int WeaponID)
{
	MESSAGE_BEGIN(MSG_ONE, MESSAGE_WEAPONLIST, NULL, PlayerEdict);
	switch (WeaponID)
	{
		case CSW_P228: { WRITE_STRING("weapon_p228");  WRITE_BYTE(9);  WRITE_BYTE(52); WRITE_BYTE(-1); WRITE_BYTE(-1); WRITE_BYTE(1); WRITE_BYTE(3); WRITE_BYTE(1); WRITE_BYTE(0); break; }
		case CSW_XM1014: { WRITE_STRING("weapon_xm1014");  WRITE_BYTE(5);  WRITE_BYTE(32); WRITE_BYTE(-1); WRITE_BYTE(-1); WRITE_BYTE(0); WRITE_BYTE(12); WRITE_BYTE(5); WRITE_BYTE(0); break; }
		case CSW_AK47: {  WRITE_STRING("weapon_ak47"); WRITE_BYTE(2);  WRITE_BYTE(90); WRITE_BYTE(-1); WRITE_BYTE(-1); WRITE_BYTE(0); WRITE_BYTE(1); WRITE_BYTE(28); WRITE_BYTE(0); break; }
		case CSW_AWP: { WRITE_STRING("weapon_awp");  WRITE_BYTE(1);  WRITE_BYTE(30); WRITE_BYTE(-1); WRITE_BYTE(-1); WRITE_BYTE(0); WRITE_BYTE(2); WRITE_BYTE(18); WRITE_BYTE(0); break; }
	}
	MESSAGE_END();
}

static void SetWeaponHUDCustom(edict_t *PlayerEdict, CWeapon &Weapon)
{
	MESSAGE_BEGIN(MSG_ONE, MESSAGE_WEAPONLIST, NULL, PlayerEdict);
	WRITE_STRING(Weapon.GModel);
	WRITE_BYTE(Weapon.AmmoID);
	WRITE_BYTE(Ammos[Weapon.AmmoID].Max);

	switch (Weapon.Type)
	{
		case WType::Pistol: { WRITE_BYTE(-1); WRITE_BYTE(-1); WRITE_BYTE(1); WRITE_BYTE(3); WRITE_BYTE(1); WRITE_BYTE(0); break; }
		case WType::Shotgun: { WRITE_BYTE(-1); WRITE_BYTE(-1); WRITE_BYTE(0); WRITE_BYTE(12); WRITE_BYTE(5); WRITE_BYTE(0); break; }
		case WType::Rifle: { WRITE_BYTE(-1); WRITE_BYTE(-1); WRITE_BYTE(0); WRITE_BYTE(1); WRITE_BYTE(28); WRITE_BYTE(0); break; }
		case WType::Sniper: { WRITE_BYTE(-1); WRITE_BYTE(-1); WRITE_BYTE(0); WRITE_BYTE(2); WRITE_BYTE(18); WRITE_BYTE(0); break; }
	}

	MESSAGE_END();
}

static void ScreenShake(edict_t *PlayerEdict, int Amplitude, int Duration, int Frequency)
{
	MESSAGE_BEGIN(MSG_ONE, MESSAGE_SCREENSHAKE, NULL, PlayerEdict);
	WRITE_SHORT(Amplitude << 12);
	WRITE_SHORT(Duration << 12);
	WRITE_SHORT(Frequency << 12);
	MESSAGE_END();
}

static void BarTime(edict_t *PlayerEdict, int Duration)
{
	MESSAGE_BEGIN(MSG_ONE, MESSAGE_BARTIME, NULL, PlayerEdict);
	WRITE_SHORT(Duration);
	MESSAGE_END();
}

static void ClientPrint(edict_t *PlayerEdict, int Type, const char *Msg)
{
	MESSAGE_BEGIN(MSG_ONE, MESSAGE_TEXTMSG, NULL, PlayerEdict);
	WRITE_BYTE(Type);
	WRITE_STRING(Msg);
	MESSAGE_END();
}

BOOL InViewCone(edict_t *PlayerEdict, Vector &Origin, BOOL Accurate)
{
	Vector LOS = (Origin - (PlayerEdict->v.origin + PlayerEdict->v.view_ofs)).Normalize();

	MAKE_VECTORS(PlayerEdict->v.v_angle);

	if (DotProduct(LOS, gpGlobals->v_forward) >= cos(PlayerEdict->v.fov * (M_PI / (Accurate ? 480 : 360))))
		return TRUE;
	else
		return FALSE;
}

BOOL InViewCone(Vector &SelfOrigin, Vector &VAngles, float FOV, Vector &Origin, BOOL Accurate)
{
	Vector LOS = (Origin - (SelfOrigin)).Normalize();

	MAKE_VECTORS(VAngles);

	if (DotProduct(LOS, gpGlobals->v_forward) >= cos(FOV * (M_PI / (Accurate ? 480 : 360))))
		return TRUE;
	else
		return FALSE;
}

void PlayerKnockback(edict_t *Edict, Vector &Origin)
{
	Vector Direction = (Edict->v.origin - Origin).Normalize();
	Direction[0] = Direction[0] * KnockbackInfo.PushPower;
	Direction[1] = Direction[1] * KnockbackInfo.PushPower;
	Direction[2] = Direction[2] * KnockbackInfo.PushPower * KnockbackInfo.JumpPower;
	Edict->v.velocity = Direction;
}

static void PerformDamage(edict_t *PlayerEdict, float Radius, float Damage, CBasePlayerWeapon *BaseWeapon, BOOL Accurate, float Knockback)
{
	BOOL Result;
	Vector MyOrigin = PlayerEdict->v.origin + PlayerEdict->v.view_ofs, Origin;
	edict_t *TargetEdict = NULL;
	TraceResult TResult;
	CBaseEntity *BaseEntity;
	entvars_t *PlayerEntVars = &PlayerEdict->v;
	entvars_t *EntityVars;

	while ((TargetEdict = FIND_ENTITY_IN_SPHERE(TargetEdict, PlayerEdict->v.origin, Radius)) != SVGame_Edicts)
	{
		if (!TargetEdict->pvPrivateData || TargetEdict == PlayerEdict)
			continue;

		if (!TargetEdict->v.takedamage)
			continue;

		BaseEntity = (CBaseEntity *)TargetEdict->pvPrivateData;
		EntityVars = EV_FROM_PD(BaseEntity);
		Origin = TargetEdict->v.origin;

		if (EntityVars->solid == SOLID_BSP)
			Origin = Origin + (TargetEdict->v.mins + TargetEdict->v.maxs) * 0.5f;

		if (!InViewCone(PlayerEdict, Origin, Accurate))
			continue;

		if (!(WEAPON_FLAGS(BaseWeapon) & WFlag::KnifeAttack_Penetration))
		{
			TRACE_LINE(MyOrigin, Origin, 0, PlayerEdict, &TResult);

			if (TResult.pHit != TargetEdict)
				continue;
		}

		Result = CALLFUNC_GAME(FN_TAKEDAMAGE, FEntity_TakeDamage, BaseEntity, PlayerEntVars, PlayerEntVars, Damage, DMG_SLASH);

		if (Result && EntityVars->flags & FL_CLIENT && (WEAPON_FLAGS(BaseWeapon) & WFlag::KnifeAttack_Knockback))
			PlayerKnockback(TargetEdict, MyOrigin);
	}
}

/* ===================================================== */
/* =================== ATTACK 2 ======================== */
/* ===================================================== */

static void Attack2_Zoom(CBasePlayer *BasePlayer, CBasePlayerWeapon *BaseWeapon, CWeapon &Weapon)
{
	int &FOV = GetPrivateData(int, BasePlayer, CBasePlayer_FOV, 5);

	switch (GetAttack2Data(int, Weapon.A2I, WA2_ZOOM_MODE))
	{
		case Zoom_Rifle:
		{
			FOV = (FOV == CS_NO_ZOOM) ? CS_AUGSG552_ZOOM : CS_NO_ZOOM;
			break;
		}
		case Zoom_SniperF:
		{
			FOV = (FOV == CS_NO_ZOOM) ? CS_FIRST_ZOOM : CS_NO_ZOOM;
			break;
		}
		case Zoom_SniperS:
		{
			FOV = (FOV == CS_NO_ZOOM) ? CS_SECOND_AWP_ZOOM : CS_NO_ZOOM;
			break;
		}
		case Zoom_SniperB:
		{
			switch (FOV)
			{
				case CS_NO_ZOOM: FOV = CS_FIRST_ZOOM; break;
				case CS_FIRST_ZOOM: FOV = CS_SECOND_AWP_ZOOM; break;
				case CS_SECOND_AWP_ZOOM: FOV = CS_NO_ZOOM; break;
			}
			break;
		}
	}

	WEAPON_INA2(BaseWeapon) = !WEAPON_INA2(BaseWeapon);

	if (!(Weapon.Flags & WFlag::Zoom_NoSound))
		CLIENT_COMMAND(ED_FROM_PD(BasePlayer), "spk weapons/zoom\n");
}

static void Attack2_Switch(CBasePlayer *BasePlayer, CBasePlayerWeapon *BaseWeapon, CWeapon &Weapon)
{
	edict_t *PlayerEdict = ED_FROM_PD(BasePlayer);

	switch (WEAPON_INA2(BaseWeapon))
	{
		case FALSE:
		{
			SendWeaponAnim(BaseWeapon, GetAttack2Data(int, Weapon.A2V, WA2_SWITCH_ANIM_A));
			GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_NextReload, 4)
				= GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_NextPrimaryAttack, 4)
				= GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_NextSecondaryAttack, 4)
				= GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_TimeWeaponIdle, 4)
				= GetAttack2Data(float, Weapon.A2V, WA2_SWITCH_ANIM_A_DURATION);

			if (!(Weapon.Flags & SwitchMode_NoText))
				CLIENT_PRINT(PlayerEdict, print_center, "Switch to Mode B");

			break;
		}
		case TRUE:
		{
			SendWeaponAnim(BaseWeapon, GetAttack2Data(int, Weapon.A2V, WA2_SWITCH_ANIM_B));
			GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_NextReload, 4)
				= GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_NextPrimaryAttack, 4)
				= GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_NextSecondaryAttack, 4)
				= GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_TimeWeaponIdle, 4)
				= GetAttack2Data(float, Weapon.A2V, WA2_SWITCH_ANIM_B_DURATION);

			if (!(Weapon.Flags & SwitchMode_NoText))
				CLIENT_PRINT(PlayerEdict, print_center, "Switch to Mode B");

			break;
		}
	}

	if (Weapon.Flags & SwitchMode_BarTime)
		BarTime(PlayerEdict, *(int *)&GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_NextPrimaryAttack, 4));

	WEAPON_INA2_DELAY(BaseWeapon) = TRUE;
}

static void Attack2_Burst(CBasePlayer *BasePlayer, CBasePlayerWeapon *BaseWeapon, CWeapon &Weapon)
{
	entvars_t *PlayerEntVars = EV_FROM_PD(BasePlayer);

	switch (WEAPON_INA2(BaseWeapon))
	{
		case FALSE:
		{
			WEAPON_INA2(BaseWeapon) = TRUE;
			ClientPrint(ENT(PlayerEntVars), HUD_PRINTCENTER, "#Switch_To_BurstFire");
			break;
		}
		case TRUE:
		{
			WEAPON_INA2(BaseWeapon) = FALSE;

			if ((1 << WEAPON_ID(BaseWeapon)) & SECONDARY_WEAPONS_BIT_SUM)
				ClientPrint(ENT(PlayerEntVars), HUD_PRINTCENTER, "#Switch_To_SemiAuto");
			else
				ClientPrint(ENT(PlayerEntVars), HUD_PRINTCENTER, "#Switch_To_FullAuto");

			break;
		}
	}

	GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_NextSecondaryAttack, 4) = 0.2f;
}

static void Attack2_MultiShot(CBasePlayerWeapon *BaseWeapon, CWeapon &Weapon)
{
	if (GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_NextPrimaryAttack, 4) > 0.0f)
		return;

	WEAPON_INA2(BaseWeapon) = TRUE;

	for (int Index = 0; Index < WEAPON_A2_OFFSET(BaseWeapon); Index++)
	{
		if (WEAPON_CLIP(BaseWeapon) > 0)
			((FN_WEAPON_PRIMARYATTACK)FWeapon_PrimaryAttack[Weapon.Type])(BaseWeapon);
	}

	GetPrivateData(int, BaseWeapon, CBasePlayerWeapon_ShotsFired, 4) = TRUE;
	GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_NextSecondaryAttack, 4) = Weapon.Delay;
	WEAPON_INA2(BaseWeapon) = FALSE;
}

static void Attack2_KnifeAttack(CBasePlayer *BasePlayer, CBasePlayerWeapon *BaseWeapon, CWeapon &Weapon)
{
	GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_NextSecondaryAttack, 4)
		= GetAttack2Data(float, Weapon.A2V, WA2_KNIFEATTACK_DELAY);
	GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_TimeWeaponIdle, 4)
		= GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_NextPrimaryAttack, 4)
		= GetAttack2Data(float, Weapon.A2V, WA2_KNIFEATTACK_DURATION);
	SendWeaponAnim(BaseWeapon, GetAttack2Data(int, Weapon.A2V, WA2_KNIFEATTACK_ANIMATION));

	WEAPON_INA2_DELAY(BaseWeapon) = TRUE;

	if (!(Weapon.Flags & WFlag::KnifeAttack_NoSound))
		EMIT_SOUND_SHORT(ED_FROM_PD(BaseWeapon), CHAN_VOICE, GetAttack2Data(const char *, Weapon.A2V, WA2_KNIFEATTACK_SOUND));
}

static void Attack2_KnifeAttackPerform(CBasePlayerWeapon *BaseWeapon)
{
	WEAPON_INA2_DELAY(BaseWeapon) = FALSE;
	GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_NextSecondaryAttack, 4) = GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_NextPrimaryAttack, 4);

	CBasePlayer *BasePlayer = GetPrivateData(CBasePlayer *, BaseWeapon, CBasePlayerItem_Player, 4);
	CWeapon &Weapon = Weapons[WEAPON_KEY(BaseWeapon)];

	if (Weapon.Flags & WFlag::KnifeAttack_ScreenShake)
		ScreenShake(ED_FROM_PD(BasePlayer), 4, 4, 4);

	PerformDamage(ED_FROM_PD(BasePlayer),
		GetAttack2Data(float, Weapon.A2V, WA2_KNIFEATTACK_RADIUS),
		(float)RANDOM_LONG(GetAttack2Data(int, Weapon.A2V, WA2_KNIFEATTACK_DAMAGE_MIN), GetAttack2Data(int, Weapon.A2V, WA2_KNIFEATTACK_DAMAGE_MAX)),
		BaseWeapon, Weapon.Flags & WFlag::KnifeAttack_Accurate, GetAttack2Data(float, Weapon.A2V, WA2_KNIFEATTACK_KNOCKBACK));
}

static void Attack2_InstaSwitch(edict_t *PlayerEdict, CBasePlayerWeapon *BaseWeapon, CWeapon &Weapon)
{
	switch (WEAPON_INA2(BaseWeapon))
	{
		case FALSE:
		{
			WEAPON_INA2(BaseWeapon) = TRUE;
			ClientPrint(PlayerEdict, HUD_PRINTCENTER, GetAttack2Data(const char *, Weapon.A2V, WA2_INSTASWITCH_NAME));
			break;
		}
		case TRUE:
		{
			WEAPON_INA2(BaseWeapon) = FALSE;
			ClientPrint(PlayerEdict, HUD_PRINTCENTER, GetAttack2Data(const char *, Weapon.A2V, WA2_INSTASWITCH_NAME2));
			break;
		}
	}

	GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_NextSecondaryAttack, 4) = 0.2f;
}

static void Attack2_ZoomCustom(CBasePlayer *BasePlayer, CBasePlayerWeapon *BaseWeapon, CWeapon &Weapon)
{
	if (WEAPON_INA2(BaseWeapon))
		GetPrivateData(int, BasePlayer, CBasePlayer_FOV, 5) = 90;
	else
		GetPrivateData(int, BasePlayer, CBasePlayer_FOV, 5) = GetAttack2Data(int, Weapon.A2V, WA2_ZOOM_CUSTOM_FOV);

	WEAPON_INA2(BaseWeapon) = !WEAPON_INA2(BaseWeapon);
	
	if (!(Weapon.Flags & WFlag::Zoom_NoSound))
		CLIENT_COMMAND(ED_FROM_PD(BasePlayer), "spk weapons/zoom\n");
}

Vector VectorToAngles(Vector &InPut)
{
	float OutPut[3];
	VEC_TO_ANGLES(InPut, OutPut);
	return OutPut;
}

cell ShootProjectileTimed(edict_t *LauncherEdict, int ProjectileID)
{
	edict_t *ProjectileEdict = CREATE_NAMED_ENTITY(MAKE_STRING(PROJECTILE_CLASS));
	CGrenade *BaseEntity = ((CGrenade *)ProjectileEdict->pvPrivateData);
	entvars_t *ProjectileEntVars = EV_FROM_PD(BaseEntity);
	entvars_t *LauncherEntVars = &LauncherEdict->v;
	CProjectile *Projectile = &Projectiles[ProjectileID];
	Vector Velocity = gpGlobals->v_forward * Projectile->Speed;

	((FN_GRENADE_SPAWN)FGrenade_Spawn)(BaseEntity);
	GetPrivateData(int, BaseEntity, CGrenade_Team, 4) = GetPrivateData(int, LauncherEdict->pvPrivateData, CBasePlayer_Team, 5);
	ProjectileEntVars->movetype = MOVETYPE_PUSHSTEP;
	ProjectileEntVars->solid = SOLID_TRIGGER;
	SET_MODEL(ProjectileEdict, Projectile->Model);
	//SET_SIZE(ProjectileEdict, ZVector, ZVector);
	SET_ORIGIN(ProjectileEdict, LauncherEntVars->origin + LauncherEntVars->view_ofs + gpGlobals->v_forward * 16);
	ProjectileEntVars->gravity = Projectile->Gravity;
	ProjectileEntVars->velocity = Velocity;
	ProjectileEntVars->angles = VectorToAngles(Velocity);
	ProjectileEntVars->owner = LauncherEdict;
	ProjectileEntVars->euser1 = ED_FROM_PD(GetPrivateData(CBasePlayerItem *, LauncherEdict->pvPrivateData, CBasePlayer_ActiveItem, 5));
	ProjectileEntVars->nextthink = gpGlobals->time + Projectile->Duration;

	PROJECTILE_TYPE(BaseEntity) = PLIMITED_TIME;
	PROJECTILE_FORWARD(BaseEntity) = Projectile->Forward;
	return NUM_FOR_EDICT(ProjectileEdict);
}

cell ShootProjectileContact(edict_t *LauncherEdict, int ProjectileID)
{
	edict_t *ProjectileEdict = CREATE_NAMED_ENTITY(MAKE_STRING(PROJECTILE_CLASS));
	CGrenade *BaseEntity = ((CGrenade *)ProjectileEdict->pvPrivateData);
	entvars_t *ProjectileEntVars = EV_FROM_PD(BaseEntity);
	entvars_t *LauncherEntVars = &LauncherEdict->v;
	CProjectile *Projectile = &Projectiles[ProjectileID];
	Vector Origin = LauncherEdict->v.origin + LauncherEdict->v.view_ofs + gpGlobals->v_forward * 16;
	Vector Angles = LauncherEdict->v.angles;
	Angles[2] += RANDOM_FLOAT(10.0f, 16.0f) * 20.0f;
	
	((FN_GRENADE_SPAWN)FGrenade_Spawn)(BaseEntity);
	GetPrivateData(int, BaseEntity, CGrenade_Team, 4) = GetPrivateData(int, LauncherEdict->pvPrivateData, CBasePlayer_Team, 5);
	ProjectileEntVars->movetype = MOVETYPE_PUSHSTEP;
	ProjectileEntVars->solid = SOLID_TRIGGER;
	SET_MODEL(ProjectileEdict, Projectile->Model);
	//SET_SIZE(ProjectileEdict, Vector(0, 0, 0), Vector(0, 0, 0));
	SET_ORIGIN(ProjectileEdict, LauncherEntVars->origin + LauncherEntVars->view_ofs + gpGlobals->v_forward * 16);
	ProjectileEntVars->gravity = Projectile->Gravity;
	ProjectileEntVars->velocity = gpGlobals->v_forward * Projectile->Speed;
	ProjectileEntVars->angles = Angles;
	ProjectileEntVars->owner = LauncherEdict;
	ProjectileEntVars->euser1 = ED_FROM_PD(GetPrivateData(CBasePlayerItem *, LauncherEdict->pvPrivateData, CBasePlayer_ActiveItem, 5));

	PROJECTILE_TYPE(BaseEntity) = PLIMITED_TOUCH;
	PROJECTILE_FORWARD(BaseEntity) = Projectile->Forward;
	return NUM_FOR_EDICT(ProjectileEdict);
}

cell ShootEffect(edict_t *LauncherEdict, int EffectID)
{
	edict_t *EffectEdict = CREATE_NAMED_ENTITY(MAKE_STRING("env_sprite"));
	CBaseEntity *BaseEntity = (CBaseEntity *)EffectEdict->pvPrivateData;
	entvars_t *EffectEntVars = &EffectEdict->v;
	CEffect *Effect = &Effects[EffectID];

	Vector Origin = LauncherEdict->v.origin + LauncherEdict->v.view_ofs + gpGlobals->v_forward * 16;
	Vector Angles = LauncherEdict->v.angles;
	Angles[2] += RANDOM_FLOAT(10.0f, 16.0f) * 20.0f;

	SET_MODEL(EffectEdict, Effect->Model);
	SET_ORIGIN(EffectEdict, Origin);
	EffectEntVars->movetype = MOVETYPE_NOCLIP;
	EffectEntVars->rendermode = kRenderTransAdd;
	EffectEntVars->renderamt = 255.0f;
	EffectEntVars->scale = 2.0f;
	EffectEntVars->angles = Angles;
	EffectEntVars->solid = SOLID_NOT;
	EffectEntVars->owner = LauncherEdict;
	EffectEntVars->framerate = 18.0f;
	EffectEntVars->nextthink = EFFECT_LAST_TIME(EffectEntVars) = gpGlobals->time;
	EffectEntVars->velocity = gpGlobals->v_forward * Effect->Speed;

	//SET_SIZE(EffectEdict, ZVector, ZVector);
	EFFECT_TYPE(BaseEntity) = ELIMITED_TIME;
	EFFECT_MAX_FRAMES(EffectEntVars) = (float)MODEL_FRAMES(EffectEntVars->modelindex) - 1;
	EFFECT_EXPIRE_TIME(BaseEntity) = Effect->Duration ? gpGlobals->time + Effect->Duration : 0.0f;
	EFFECT_FORWARD(BaseEntity) = Effect->Forward;
	return NUM_FOR_EDICT(EffectEdict);
}

void SetClientKeyValue(int Index, char *InfoBuffer, const char *Key, const char *Value)
{
	SET_META_RESULT(MRES_IGNORED);

	if (SpecialBot_VTable)
		return;

	edict_t *PlayerEdict = EDICT_FOR_NUM(Index);

	if ((PlayerEdict->v.flags & FL_FAKECLIENT) != FL_FAKECLIENT)
	{
		const char *Auth = GETPLAYERAUTHID(PlayerEdict);

		if (Auth && (strcmp(Auth, "BOT") != 0))
			return;
	}

	if (strcmp(Key, "*bot") != 0 || strcmp(Value, "1") != 0)
		return;

	SpecialBot_VTable = *((void ***)((char *)PlayerEdict->pvPrivateData));
	HookEntityFWByVTable(SpecialBot_VTable, EO_TakeDamage, (V *)Player_TakeDamage, &FPlayerBot_TakeDamage);
}

static int HOOK_OFFSETS[] =
{
	EO_Spawn, EO_Item_AddToPlayer, EO_Item_Deploy,
	EO_Weapon_PrimaryAttack, EO_Weapon_SecondaryAttack, EO_Weapon_Reload,
	EO_Item_ItemPostFrame, EO_Weapon_WeaponIdle, EO_Item_Holster,
	EO_Item_GetMaxSpeed, EO_Weapon_ExtractAmmo, EO_Weapon_SendWeaponAnim,
};

static void *HOOK_FUNCS[][MAX_WEAPON_TYPES] =
{
	{ (V *)Weapon_SpawnPistol, (V *)Weapon_SpawnShotgun, (V *)Weapon_SpawnRifle, (V *)Weapon_SpawnSniper },
	{ (V *)Weapon_AddToPlayer, (V *)Weapon_AddToPlayer, (V *)Weapon_AddToPlayer, (V *)Weapon_AddToPlayer },
	{ (V *)Weapon_Deploy, (V *)Weapon_Deploy, (V *)Weapon_Deploy, (V *)Weapon_Deploy },
	{ (V *)Weapon_PrimaryAttack, (V *)Weapon_PrimaryAttack, (V *)Weapon_PrimaryAttack, (V *)Weapon_PrimaryAttack },
	{ (V *)Weapon_SecondaryAttack, (V *)Weapon_SecondaryAttack, (V *)Weapon_SecondaryAttack, (V *)Weapon_SecondaryAttack },
	{ (V *)Weapon_Reload, (V *)Weapon_ReloadShotgun, (V *)Weapon_Reload, (V *)Weapon_Reload },
	{ (V *)Weapon_PostFrame, (V *)Weapon_PostFrame, (V *)Weapon_PostFrame, (V *)Weapon_PostFrame },
	{ (V *)Weapon_Idle, (V *)Weapon_IdleShotgun, (V *)Weapon_Idle, (V *)Weapon_Idle },
	{ (V *)Weapon_Holster, (V *)Weapon_Holster, (V *)Weapon_Holster, (V *)Weapon_Holster },
	{ (V *)Weapon_GetMaxSpeed, (V *)Weapon_GetMaxSpeed, (V *)Weapon_GetMaxSpeed, (V *)Weapon_GetMaxSpeed },
	{ (V *)Weapon_ExtractAmmo, (V *)Weapon_ExtractAmmo, (V *)Weapon_ExtractAmmo, (V *)Weapon_ExtractAmmo },
	{ (V *)Weapon_SendWeaponAnim, (V *)Weapon_SendWeaponAnim, (V *)Weapon_SendWeaponAnim, (V *)Weapon_SendWeaponAnim },
};

static void *HOOK_STORE[] =
{
	FWeapon_Spawn,
	FWeapon_AddToPlayer,
	FWeapon_Deploy,
	FWeapon_PrimaryAttack,
	FWeapon_SecondaryAttack,
	FWeapon_Reload,
	FWeapon_PostFrame,
	FWeapon_Idle,
	FWeapon_Holster,
	FWeapon_GetMaxSpeed,
	FWeapon_ExtractAmmo,
	FWeapon_SendWeaponAnim,
};

BOOL DispatchSpawn(edict_t *Entity)
{
	SET_META_RESULT(MRES_IGNORED);

	if (EntityHooked)
		return 0;

	const char *WeaponName;

	for (int Index = 0; Index < MAX_WEAPON_TYPES; Index++)
	{
		WeaponName = WEAPON_CLASS[WEAPON_TYPE_ID[Index]];

		for (int OfIndex = 0; OfIndex < MAX_HOOKS; OfIndex++)
			HookEntityFW(WeaponName, HOOK_OFFSETS[OfIndex], (V *)HOOK_FUNCS[OfIndex][Index], ((V **)HOOK_STORE[OfIndex] + Index));
	}

	HookEntityFW(PLAYER_CLASS, EO_TakeDamage, (V *)Player_TakeDamage, &FPlayer_TakeDamage);
	HookEntityFW(PLAYER_CLASS, EO_GiveAmmo, (V *)Player_GiveAmmo, &FPlayer_GiveAmmo);
	HookEntityFW("worldspawn", EO_TraceAttack, (V *)TraceAttack, &FTraceAttackEntity);
	HookEntityFW("func_wall", EO_TraceAttack, (V *)TraceAttack, &FTraceAttackEntity);
	HookEntityFW("func_tank", EO_TraceAttack, (V *)TraceAttack, &FTraceAttackEntity);
	HookEntityFW("func_breakable", EO_TraceAttack, (V *)TraceAttack, &FTraceAttackEntity);
	HookEntityFW("func_door", EO_TraceAttack, (V *)TraceAttack, &FTraceAttackEntity);
	HookEntityFW("func_door_rotating", EO_TraceAttack, (V *)TraceAttack, &FTraceAttackEntity);
	HookEntityFW(PLAYER_CLASS, EO_TraceAttack, (V *)TraceAttackPlayer, &FTraceAttackPlayer);
	HookEntityFW(PROJECTILE_CLASS, EO_Think, (V *)ProjectileThink, &FProjectileThink);
	HookEntityFW(PROJECTILE_CLASS, EO_Touch, (V *)ProjectileTouch, &FProjectileTouch);
	HookEntityFW(EFFECT_CLASS, EO_Think, (V *)EffectThink, &FEffectThink);

	FPlayerKilled = GetEntityFW(PLAYER_CLASS, EO_Killed);
	FGrenade_Spawn = GetEntityFW(PROJECTILE_CLASS, EO_Spawn);
	FEntity_TakeDamage = GetEntityFW("func_wall", EO_TakeDamage);
	FWeapon_PlayEmptySound = GetEntityFW(WEAPON_CLASS[CSW_P228], EO_Weapon_PlayEmptySound);
	FWeapon_ResetEmptySound = GetEntityFW(WEAPON_CLASS[CSW_P228], EO_Weapon_ResetEmptySound);

	EntityHooked = TRUE;
	return 0;
}

void UpdateAmmoList()
{
	for (int Index = 0; Index < AMMO_MAX_TYPES; Index++)
		Ammos.Append(DEFAULT_AMMOS[Index]);
}

BOOL DispatchSpawn_Post(edict_s *Entity)
{
	SET_META_RESULT(MRES_IGNORED);

	if (Initialized)
		return 0;
	
	LoadAmmos();
	LoadWeapons();
	PrecacheModule();
	CanPrecache = FALSE;
	Initialized = TRUE;

	for (int Index = 0; Index < AmmoCount; Index++)
	{
		CAmmo &Ammo = Ammos[Index];
		
		if (!Ammo.Name)
		{
			char DefName[32];
			sprintf(DefName, "Ammo%i", AmmoCount);
			Ammo.Name = STRING(ALLOC_STRING(DefName));
		}

		CheckAmmo(Ammo, Ammo.Name);
	}

	for (int Index = 0; Index < WeaponCount; Index++)
	{
		CWeapon &Weapon = Weapons[Index];

		if (!Weapon.Model)
		{
			const char *Name = Weapon.Name;
			LOG_CONSOLE(PLID, "[CSWM] Found Weapon Without Model, Ignoring... [%s]", Name ? Name : "<Unnamed>");
			continue;
		}

		CheckWeapon(Weapon);
		RecordWeaponDurationList(Weapons[Index]);
	}

	return 0;
}

void ServerActivate(edict_t *Worldspawn, int MaxEdicts, int MaxPlayers)
{
	SET_META_RESULT(MRES_IGNORED);
	SVGame_Edicts = Worldspawn;
	entvars_t *EntityVars = &Worldspawn->v;
	byte *PrivateData = (byte *)Worldspawn->pvPrivateData;

	for (size_t Index = 0; Index < 0xFFF; ++Index)
	{
		if (*(entvars_t **)(PrivateData + Index) == EntityVars)
		{
			PEV_Offset = Index;
			return;
		}
	}

	LOG_CONSOLE(PLID, "[CSWM] Entity Vars Offset Is Invalid!");
}

void ServerDeactivate_Post(void)
{
	Initialized = FALSE;
	CanPrecache = TRUE;
	ClearWeapons = TRUE;
	AmmoCount = AMMO_MAX_TYPES;
	Ammos.Clear();
	Projectiles.Clear();
	UpdateAmmoList();
	RETURN_META(MRES_IGNORED);
}

void OnPluginsUnloaded(void)
{
	for (int Index = 0; Index < WeaponCount; Index++)
	{
		CWeapon &Weapon = Weapons[Index];

		if (Weapon.A2V)
			delete[](int *)Weapon.A2V;
	}

	WeaponCount = 0;
	Weapons.Clear();
	ClearWeapons = FALSE;

	for (int Index = gpGlobals->maxClients + 1; Index < gpGlobals->maxEntities; Index++)
	{
		edict_t *Edict = EDICT_FOR_NUM(Index);

		if (!Edict->pvPrivateData)
			continue;

		CBaseEntity *BaseEntity = (CBaseEntity *)Edict->pvPrivateData;

		if (CUSTOM_WEAPON(BaseEntity))
			CUSTOM_WEAPON(BaseEntity) = 0;
	}
}