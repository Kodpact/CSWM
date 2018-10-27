#include "Module.h"

//#pragma warning (disable : 4701)
//#pragma warning (disable : 4996)

edict_t *SVGame_Edicts;

List<CWeapon> Weapons;
List<CAmmo> Ammos;
List<CProjectile> Projectiles;
List<CEffect> Effects;

StringHashMap AnimHashMap;

BOOL Activated = FALSE;
BOOL Initialized = FALSE;
BOOL EntityHooked = FALSE;
BOOL PrecachedWeapons = FALSE;
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
void *FPlayer_TakeDamage, *FPlayerBot_TakeDamage;
void *FPlayer_GiveAmmo;
void *FWeaponBox_Spawn;
void *FTraceAttackEntity;
void *FTraceAttackPlayer;
void *FPlayerKilled;
void *FProjectileThink;
void *FProjectileTouch;
void *FEffectThink;

static const Vector ZVector = { 0.0f, 0.0f, 0.0f };

static void SetWeaponHUD(edict_t *Player, int WeaponID);
static void SetWeaponHUDCustom(edict_t *Player, CWeapon &Weapon);
static void ScreenShake(edict_t *PlayerEdict, int Amplitude, int Duration, int Frequency);
static void ClientPrint(edict_t *PlayerEdict, int Type, const char *Msg);
static void BarTime(edict_t *PlayerEdict, int Duration);
static void GiveAmmo2(edict_t *PlayerEdict, CBasePlayer *BasePlayer, int AmmoIndex, CAmmo *Ammo);
static void TraceAttackContinue(CBaseEntity *BaseEntity, entvars_t *AttackerVars, TraceResult *TResult);
static int __fastcall Player_GiveAmmo(CBasePlayer *BasePlayer, int, int Amount, const char *Name, int Max);

const CAmmo DEFAULT_AMMOS[]
{
	{ NULL, NULL, NULL, "\0" },
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

cvar_t CVar_Version = { "cswm_version", MODULE_VERSION, FCVAR_ARCHIVE | FCVAR_SERVER, 0.0f, nullptr };
cvar_t CVar_Log = { "cswm_log", "0", FCVAR_ARCHIVE | FCVAR_SERVER, 0.0f, nullptr };
cvar_t CVar_AMap = { "cswm_amap", "1", FCVAR_ARCHIVE | FCVAR_SERVER, 0.0f, nullptr };
cvar_t *CVar_LogPointer;
cvar_t *CVar_AMapPointer;
cvar_t *CVar_CheatsPointer;

void GameDLLInit_Post(void)
{
	SET_META_RESULT(MRES_IGNORED);

	CVAR_REGISTER(&CVar_Version);
	CVAR_REGISTER(&CVar_Log);
	CVAR_REGISTER(&CVar_AMap);

	CVar_LogPointer = CVAR_GET_POINTER("cswm_log");
	CVar_AMapPointer = CVAR_GET_POINTER("cswm_amap");
	CVar_CheatsPointer = CVAR_GET_POINTER("sv_cheats");
}

static void __fastcall Weapon_SpawnPistol(CBasePlayerWeapon *BaseWeapon, int)
{
	WEAPON_FID(BaseWeapon) = WType::Pistol;
	static_cast<void(__fastcall *)(CBasePlayerWeapon *)>(FWeapon_Spawn[WType::Pistol])(BaseWeapon);
}

static void __fastcall Weapon_SpawnShotgun(CBasePlayerWeapon *BaseWeapon, int)
{
	WEAPON_FID(BaseWeapon) = WType::Shotgun;
	static_cast<void(__fastcall *)(CBasePlayerWeapon *)>(FWeapon_Spawn[WType::Shotgun])(BaseWeapon);
}

static void __fastcall Weapon_SpawnRifle(CBasePlayerWeapon *BaseWeapon, int)
{
	WEAPON_FID(BaseWeapon) = WType::Rifle;
	static_cast<void(__fastcall *)(CBasePlayerWeapon *)>(FWeapon_Spawn[WType::Rifle])(BaseWeapon);
}

static void __fastcall Weapon_SpawnSniper(CBasePlayerWeapon *BaseWeapon, int)
{
	WEAPON_FID(BaseWeapon) = WType::Sniper;
	static_cast<void(__fastcall *)(CBasePlayerWeapon *)>(FWeapon_Spawn[WType::Sniper])(BaseWeapon);
}

static BOOL __fastcall Weapon_AddToPlayer(CBasePlayerWeapon *BaseWeapon, int, CBasePlayer *BasePlayer)
{
	static_cast<BOOL(__fastcall *)(CBasePlayerWeapon *, int, CBasePlayer *)>(FWeapon_AddToPlayer[WEAPON_FID(BaseWeapon)])(BaseWeapon, NULL, BasePlayer);

	edict_t *PlayerEdict = ENT(BasePlayer->pev);

	if (!CUSTOM_WEAPON(BaseWeapon))
		SetWeaponHUD(PlayerEdict, BaseWeapon->m_iId);
	else
	{
		CWeapon &Weapon = Weapons[WEAPON_KEY(BaseWeapon)];
		SetWeaponHUDCustom(PlayerEdict, Weapon);

		if (!(Weapon.Flags & WFlag::SoloClip))
			BaseWeapon->m_iPrimaryAmmoType = Weapon.AmmoID;
	}

	return TRUE;
}

static BOOL __fastcall Weapon_Deploy(CBasePlayerWeapon *BaseWeapon, int)
{
	static_cast<BOOL(__fastcall *)(CBasePlayerWeapon *)>(FWeapon_Deploy[WEAPON_FID(BaseWeapon)])(BaseWeapon);

	if (!CUSTOM_WEAPON(BaseWeapon))
		return TRUE;

	CWeapon &Weapon = Weapons[WEAPON_KEY(BaseWeapon)];
	entvars_t *PlayerEntVars = BaseWeapon->m_pPlayer->pev;
	PlayerEntVars->viewmodel = Weapon.VModel;
	PlayerEntVars->weaponmodel = Weapon.PModel;
	BOOL SwitchON = (Weapon.A2I == A2_Switch && WEAPON_INA2(BaseWeapon)) ? TRUE : FALSE;

	BaseWeapon->m_pPlayer->m_flNextAttack = BaseWeapon->m_flNextSecondaryAttack = BaseWeapon->m_flNextPrimaryAttack = SwitchON ? Weapon.A2V->WA2_SWITCH_ANIM_DRAW_DURATION : Weapon.Deploy;
	SwitchON = SwitchON ? Weapon.A2V->WA2_SWITCH_ANIM_DRAW : Weapon.AnimD;
	BaseWeapon->m_flTimeWeaponIdle = Weapon.DurationList.Get(SwitchON);

	SendWeaponAnim(BaseWeapon, SwitchON);

	if (Weapon.Forwards[WForward::DeployPost])
		MF_ExecuteForward(Weapon.Forwards[WForward::DeployPost], NUM_FOR_EDICT(ENT(BaseWeapon->pev)));

	return TRUE;
}

#pragma warning (disable : 4701)

static void __fastcall Weapon_PrimaryAttack(CBasePlayerWeapon *BaseWeapon, int)
{
	if (!CUSTOM_WEAPON(BaseWeapon))
		return static_cast<void(__fastcall *)(CBasePlayerWeapon *)>(FWeapon_PrimaryAttack[WEAPON_FID(BaseWeapon)])(BaseWeapon);

	BOOL InAttack2;
	int Clip, FOV, A2I;
	entvars_t *PlayerEntVars;
	Vector PunchAngleOld;
	CWeapon &Weapon = Weapons[WEAPON_KEY(BaseWeapon)];
	InAttack2 = WEAPON_INA2(BaseWeapon);
	A2I = InAttack2 ? Weapon.A2I : A2_None;

	if (A2I == A2_AutoPistol && BaseWeapon->m_flNextPrimaryAttack > 0.0f)
		return;

	PlayerEntVars = BaseWeapon->m_pPlayer->pev;
	Clip = BaseWeapon->m_iClip;

	if (BaseWeapon->m_fInSpecialReload == 1 && !BaseWeapon->m_iClip)
	{
		BaseWeapon->Reload();
		return;
	}

	if (Weapon.Flags & WFlag::AutoSniper)
		FOV = BaseWeapon->m_pPlayer->m_iFOV;

	if (!A2I && !BaseWeapon->m_iClip)
	{
		BaseWeapon->PlayEmptySound();
		BaseWeapon->m_flNextPrimaryAttack = 0.2f;
		return;
	}

	if ((Weapon.Forwards[WForward::PrimaryAttackPre]) && MF_ExecuteForward(Weapon.Forwards[WForward::PrimaryAttackPre], NUM_FOR_EDICT(ENT(BaseWeapon->pev))) > WReturn::IGNORED)
		return;

	PunchAngleOld = PlayerEntVars->punchangle;
	static_cast<void(__fastcall *)(CBasePlayerWeapon *)>(FWeapon_PrimaryAttack[WEAPON_FID(BaseWeapon)])(BaseWeapon);

	if ((Weapon.A2I == A2_Burst || Weapon.A2I == A2_MultiShot) && InAttack2)
		BaseWeapon->m_iShotsFired = FALSE;

	if (Weapon.Flags & AutoSniper)
		BaseWeapon->m_pPlayer->m_iFOV = FOV;

	if (Clip <= BaseWeapon->m_iClip)
		return;

	if ((Weapon.Forwards[WForward::PrimaryAttackPrePost]) && MF_ExecuteForward(Weapon.Forwards[WForward::PrimaryAttackPrePost], NUM_FOR_EDICT(ENT(BaseWeapon->pev))) > WReturn::IGNORED)
		return;

	Vector PunchAngle = PlayerEntVars->punchangle;
	VectorSub(PunchAngle, PunchAngleOld, PunchAngle);
	
	float Delay, Recoil;
	int Anim = NULL;

	switch (A2I)
	{
		default: Delay = Weapon.Delay; Recoil = Weapon.Recoil; break;
		case A2_Switch: Delay = Weapon.A2V->WA2_SWITCH_DELAY; Anim = Weapon.A2V->WA2_SWITCH_ANIM_SHOOT; Recoil = Weapon.A2V->WA2_SWITCH_RECOIL;  break;
		case A2_AutoPistol: Delay = Weapon.A2V->WA2_AUTOPISTOL_DELAY; Anim = Weapon.A2V->WA2_AUTOPISTOL_ANIM; Recoil = Weapon.A2V->WA2_AUTOPISTOL_RECOIL; break;
		case A2_Burst: Delay = 0.5f;  Recoil = Weapon.Recoil; break;
		case A2_MultiShot: Delay = Weapon.Delay; Recoil = Weapon.Recoil * 1.5f; break;
		case A2_InstaSwitch: Delay = Weapon.A2V->WA2_INSTASWITCH_DELAY; Recoil = Weapon.A2V->WA2_INSTASWITCH_RECOIL; Anim = Weapon.A2V->WA2_INSTASWITCH_ANIM_SHOOT;
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
	BaseWeapon->m_flNextPrimaryAttack = Delay;
	BaseWeapon->m_flNextSecondaryAttack = 0.1f;
	BaseWeapon->m_flTimeWeaponIdle = (A2I == A2_Switch) ? Weapon.A2V->WA2_SWITCH_ANIM_SHOOT_DURATION : Weapon.DurationList.Get(Anim);

	EMIT_SOUND_SHORT(ENT(PlayerEntVars), CHAN_WEAPON, A2I == A2_Switch ? Weapon.A2V->WA2_SWITCH_FSOUND : Weapon.FireSound);

	switch (A2I)
	{
		case A2_None: break;
		case A2_Burst:
		{
			WEAPON_CURBURST(BaseWeapon)--;

			if (WEAPON_INBURST(BaseWeapon))
				break;

			BaseWeapon->m_flLastFire = gpGlobals->time;

			WEAPON_INBURST(BaseWeapon) = TRUE;
			WEAPON_CURBURST(BaseWeapon) = WEAPON_A2_OFFSET(BaseWeapon);
			break;
		}
		case A2_AutoPistol:
		{
			if (InAttack2)
				ScreenShake(ENT(BaseWeapon->m_pPlayer->pev), 4, 3, 7);

			break;
		}
	}

	if (Weapon.Forwards[WForward::PrimaryAttackPost])
		MF_ExecuteForward(Weapon.Forwards[WForward::PrimaryAttackPost], NUM_FOR_EDICT(ENT(BaseWeapon->pev)));

	if (Weapon.Flags & WFlag::AutoReload)
		BaseWeapon->Reload();
}

static void __fastcall Weapon_SecondaryAttack(CBasePlayerWeapon *BaseWeapon, int)
{
	if (!CUSTOM_WEAPON(BaseWeapon))
		static_cast<void(__fastcall *)(CBasePlayerWeapon *)>(FWeapon_SecondaryAttack[WEAPON_FID(BaseWeapon)])(BaseWeapon);
}

static void __fastcall Weapon_Reload(CBasePlayerWeapon *BaseWeapon, int CustomReload)
{
	if (!CUSTOM_WEAPON(BaseWeapon))
		return static_cast<void(__fastcall *)(CBasePlayerWeapon *)>(FWeapon_Reload[WEAPON_FID(BaseWeapon)])(BaseWeapon);

	CWeapon &Weapon = Weapons[WEAPON_KEY(BaseWeapon)];

	if (CUSTOM_WEAPON(BaseWeapon) && Weapon.Forwards[WForward::ReloadPre] && MF_ExecuteForward(Weapon.Forwards[WForward::ReloadPre], NUM_FOR_EDICT(ENT(BaseWeapon->pev))) > WReturn::IGNORED)
		return;

	static_cast<void(__fastcall *)(CBasePlayerWeapon *)>(FWeapon_Reload[CustomReload ? WType::Rifle : WEAPON_FID(BaseWeapon)])(BaseWeapon);

	if (!BaseWeapon->m_pPlayer->m_rgAmmo[Weapon.AmmoID] || BaseWeapon->m_iClip == Weapon.Clip)
		return;

	BOOL SwitchON = (Weapon.A2I == A2_Switch && WEAPON_INA2(BaseWeapon));

	BaseWeapon->m_pPlayer->m_iFOV = CS_NO_ZOOM;
	BaseWeapon->m_flNextSecondaryAttack = BaseWeapon->m_pPlayer->m_flNextAttack = SwitchON ? Weapon.A2V->WA2_SWITCH_ANIM_RELOAD_DURATION : Weapon.Reload;
	SwitchON = SwitchON ? Weapon.A2V->WA2_SWITCH_ANIM_RELOAD : Weapon.AnimR;
	BaseWeapon->m_flTimeWeaponIdle = Weapon.DurationList.Get(SwitchON);
	BaseWeapon->m_fInReload = TRUE;
	SendWeaponAnim(BaseWeapon, SwitchON);

	if (Weapon.Forwards[WForward::ReloadPost])
		MF_ExecuteForward(Weapon.Forwards[WForward::ReloadPost], NUM_FOR_EDICT(ENT(BaseWeapon->pev)));
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
			Result = NULL;
	}
	else Result = LookupSequence(GET_MODEL_PTR(ENT(BasePlayer->pev)), AnimExtension);

	return Result;
}

static void __fastcall Weapon_ReloadShotgun(CBasePlayerWeapon *BaseWeapon, int)
{
	if (!CUSTOM_WEAPON(BaseWeapon))
	{
		return static_cast<void(__fastcall *)(CBasePlayerWeapon *)>(FWeapon_Reload[WType::Shotgun])(BaseWeapon);
	}
	else
	{
		CWeapon &Weapon = Weapons[WEAPON_KEY(BaseWeapon)];

		if (Weapon.AnimR >= WShotgunReloadType::RifleStyle)
			return Weapon_Reload(BaseWeapon, TRUE);

		if (BaseWeapon->m_pPlayer->m_rgAmmo[BaseWeapon->m_iPrimaryAmmoType] <= 0)
			return;

		if (BaseWeapon->m_iClip >= Weapon.Clip)
			return;

		if (BaseWeapon->m_fInSpecialReload == 0)
		{
			SetAnimation(ENT(BaseWeapon->m_pPlayer->pev), GetPlayerSequence(BaseWeapon->m_pPlayer, SHOTGUN_RELOAD_ANIM[BaseWeapon->m_pPlayer->pev->bInDuck]), ACT_RELOAD);
			SendWeaponAnim(BaseWeapon, XM1014_START_RELOAD);

			BaseWeapon->m_fInSpecialReload = 1;
			BaseWeapon->m_flNextPrimaryAttack = BaseWeapon->m_flNextSecondaryAttack = BaseWeapon->m_flTimeWeaponIdle = BaseWeapon->m_pPlayer->m_flNextAttack = 0.55f;
		}
		else if (BaseWeapon->m_fInSpecialReload == 1)
		{
			if (BaseWeapon->m_flTimeWeaponIdle > 0.0f)
				return;

			BaseWeapon->m_fInSpecialReload = 2;

			if (!(Weapon.Flags & WFlag::ShotgunCustomReloadSound))
				EMIT_SOUND(ENT(BaseWeapon->m_pPlayer->pev), CHAN_ITEM, RANDOM_LONG(0, 1) ? "weapons/reload1.wav" : "weapons/reload3.wav", VOL_NORM, ATTN_NORM, 0, 85 + RANDOM_LONG(0, 31));

			SendWeaponAnim(BaseWeapon, XM1014_RELOAD);
			BaseWeapon->m_flNextReload = Weapon.Reload;
			BaseWeapon->m_flTimeWeaponIdle = Weapon.DurationList.Get(XM1014_RELOAD);
		}
		else
		{
			BaseWeapon->m_iClip++;
			BaseWeapon->m_pPlayer->m_rgAmmo[BaseWeapon->m_iPrimaryAmmoType]--;
			BaseWeapon->m_fInSpecialReload = 1;
		}
	}
}

static void __fastcall Weapon_PostFrame_SecondaryAttack_Pre(CBasePlayerWeapon *BaseWeapon, int AttackIndex)
{
	BOOL InAttack2 = WEAPON_INA2_DELAY(BaseWeapon);

	switch (AttackIndex)
	{
		case A2_None: break;
		case A2_Switch:
		{
			if (InAttack2 && BaseWeapon->m_flNextPrimaryAttack <= 0.0f)
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
				if (!BaseWeapon->m_fInReload && BaseWeapon->m_iClip && gpGlobals->time - 0.025f > BaseWeapon->m_flLastFire)
					return Weapon_PrimaryAttack(BaseWeapon, NULL);
			}

			if ((1 << BaseWeapon->m_iId) & SECONDARY_WEAPONS_BIT_SUM)
				BaseWeapon->m_iShotsFired = TRUE;

			WEAPON_CURBURST(BaseWeapon) = NULL;
			WEAPON_INBURST(BaseWeapon) = FALSE;
			break;
		}
		case A2_AutoPistol:
		{
			if (WEAPON_INA2(BaseWeapon))
				BaseWeapon->PrimaryAttack();

			break;
		}
		case A2_KnifeAttack:
		{
			if (InAttack2 && BaseWeapon->m_flNextSecondaryAttack <= 0.0f)
				Attack2_KnifeAttackPerform(BaseWeapon);

			break;
		}
	}
}

static void __fastcall Weapon_PostFrame_SecondaryAttack_Post(CBasePlayerWeapon *BaseWeapon)
{
	CBasePlayer *BasePlayer = BaseWeapon->m_pPlayer;

	if (BasePlayer->pev->button & IN_ATTACK2)
	{
		if (BaseWeapon->m_flNextSecondaryAttack > 0.0f)
			return;

		BasePlayer->pev->button &= ~IN_ATTACK2;
		CWeapon &Weapon = Weapons[WEAPON_KEY(BaseWeapon)];

		if ((Weapon.Forwards[WForward::SecondaryAttackPre]) && MF_ExecuteForward(Weapon.Forwards[WForward::SecondaryAttackPre], NUM_FOR_EDICT(ENT(BaseWeapon->pev))) > WReturn::IGNORED)
			return;

		switch (Weapon.A2I)
		{
			case A2_Zoom: Attack2_Zoom(BasePlayer, BaseWeapon, Weapon.A2V->WA2_ZOOM_MODE); BaseWeapon->m_flNextSecondaryAttack = 0.3f; break;
			case A2_Switch: Attack2_Switch(BasePlayer, BaseWeapon, Weapon); break;
			case A2_Burst: Attack2_Burst(BasePlayer, BaseWeapon, Weapon); break;
			case A2_AutoPistol: WEAPON_INA2(BaseWeapon) = TRUE; BaseWeapon->m_iShotsFired = FALSE; break;
			case A2_MultiShot: Attack2_MultiShot(BaseWeapon, Weapon); break;
			case A2_KnifeAttack: Attack2_KnifeAttack(BasePlayer, BaseWeapon, Weapon); break;
			case A2_InstaSwitch: Attack2_InstaSwitch(ENT(BasePlayer->pev), BaseWeapon, Weapon); break;
			case A2_ZoomCustom: Attack2_ZoomCustom(BasePlayer, BaseWeapon, Weapon.A2V->WA2_ZOOM_CUSTOM_FOV);
		}

		if (Weapon.Forwards[WForward::SecondaryAttackPost])
			MF_ExecuteForward(Weapon.Forwards[WForward::SecondaryAttackPost], NUM_FOR_EDICT(ENT(BaseWeapon->pev)));
	}
	else if (WEAPON_A2(BaseWeapon) == A2_AutoPistol && WEAPON_INA2(BaseWeapon))
		WEAPON_INA2(BaseWeapon) = FALSE;
}

static void __fastcall CheckWeaponAttack2(CBasePlayerWeapon *BaseWeapon)
{
	if (WEAPON_A2(BaseWeapon))
	{
		Weapon_PostFrame_SecondaryAttack_Pre(BaseWeapon, WEAPON_A2(BaseWeapon));
		Weapon_PostFrame_SecondaryAttack_Post(BaseWeapon);
	}
}

static void __fastcall Weapon_PostFrame(CBasePlayerWeapon *BaseWeapon, int)
{
	if (CUSTOM_WEAPON(BaseWeapon))
	{
		if (BaseWeapon->m_fInReload && BaseWeapon->m_flNextPrimaryAttack <= 0.0f)
		{
			int MaxClip = Weapons[WEAPON_KEY(BaseWeapon)].Clip;
			int Add = min(MaxClip - BaseWeapon->m_iClip, BaseWeapon->m_pPlayer->m_rgAmmo[BaseWeapon->m_iPrimaryAmmoType]);
			BaseWeapon->m_iClip += Add;
			BaseWeapon->m_pPlayer->m_rgAmmo[BaseWeapon->m_iPrimaryAmmoType] -= Add;
			BaseWeapon->m_fInReload = FALSE;
		}

		CheckWeaponAttack2(BaseWeapon);
	}

	static_cast<void(__fastcall *)(CBasePlayerWeapon *)>(FWeapon_PostFrame[WEAPON_FID(BaseWeapon)])(BaseWeapon);
}

static void __fastcall Weapon_Idle(CBasePlayerWeapon *BaseWeapon, int)
{
	if (CUSTOM_WEAPON(BaseWeapon))
	{
		CWeapon &Weapon = Weapons[WEAPON_KEY(BaseWeapon)];

		if (BaseWeapon->m_flTimeWeaponIdle <= 0.0f)
		{
			int Animation;

			if ((Weapon.A2I == A2_Switch && WEAPON_INA2(BaseWeapon)))
				Animation = Weapon.A2V->WA2_SWITCH_ANIM_IDLE;
			else if (Weapon.Flags & WFlag::CustomIdleAnim)
				Animation = BaseWeapon->m_iFamasShotsFired;
			else
				Animation = 0;

			SendWeaponAnim(BaseWeapon, Animation);
			BaseWeapon->m_flTimeWeaponIdle = Weapon.DurationList[Animation];
		}
	}
	else static_cast<void(__fastcall *)(CBasePlayerWeapon *)>(FWeapon_Idle[Rifle])(BaseWeapon);
}

static void __fastcall Weapon_IdleShotgun(CBasePlayerWeapon *BaseWeapon, int)
{
	if (CUSTOM_WEAPON(BaseWeapon))
	{
		CWeapon &Weapon = Weapons[WEAPON_KEY(BaseWeapon)];

		if (Weapon.AnimR)
			return Weapon_Idle(BaseWeapon, NULL);

		BaseWeapon->ResetEmptySound();
		BaseWeapon->m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

		if (((CXM1014 *)BaseWeapon)->m_flPumpTime && ((CXM1014 *)BaseWeapon)->m_flPumpTime < 0.0f)
			((CXM1014 *)BaseWeapon)->m_flPumpTime = 0;

		if (BaseWeapon->m_flTimeWeaponIdle < 0.0f)
		{
			if (BaseWeapon->m_iClip == 0 && BaseWeapon->m_fInSpecialReload == 0 && BaseWeapon->m_pPlayer->m_rgAmmo[BaseWeapon->m_iPrimaryAmmoType])
			{
				BaseWeapon->Reload();
			}
			else if (BaseWeapon->m_fInSpecialReload != 0)
			{
				if (BaseWeapon->m_iClip != Weapon.Clip && BaseWeapon->m_pPlayer->m_rgAmmo[BaseWeapon->m_iPrimaryAmmoType])
				{
					BaseWeapon->Reload();
				}
				else
				{
					SendWeaponAnim(BaseWeapon, XM1014_PUMP);

					BaseWeapon->m_fInSpecialReload = 0;
					BaseWeapon->m_flTimeWeaponIdle = 1.5f;
				}
			}
			else
			{
				SendWeaponAnim(BaseWeapon, XM1014_IDLE);
			}
		}
	}
	else static_cast<void(__fastcall *)(CBasePlayerWeapon *)>(FWeapon_Idle[WType::Shotgun])(BaseWeapon);
}

static void __fastcall Weapon_Holster(CBasePlayerWeapon *BaseWeapon, int, int SkipLocal)
{
	static_cast<void(__fastcall *)(CBasePlayerWeapon *, int, int)>(FWeapon_Holster[WEAPON_FID(BaseWeapon)])(BaseWeapon, NULL, SkipLocal);

	if (!CUSTOM_WEAPON(BaseWeapon))
		return;

	CWeapon &Weapon = Weapons[WEAPON_KEY(BaseWeapon)];

	if (Weapon.A2I == A2_KnifeAttack && WEAPON_INA2_DELAY(BaseWeapon))
		STOP_SOUND(ENT(BaseWeapon->pev), CHAN_VOICE, Weapon.A2V->WA2_KNIFEATTACK_SOUND);

	if (Weapon.Flags & SwitchMode_BarTime && WEAPON_INA2(BaseWeapon))
		BarTime(ENT(BaseWeapon->m_pPlayer->pev), 0);

	WEAPON_INA2_DELAY(BaseWeapon) = FALSE;

	if (Weapon.Type == WType::Shotgun && Weapon.AnimR < WShotgunReloadType::RifleStyle)
		BaseWeapon->m_fInSpecialReload = NULL;

	if ((Weapon.Forwards[WForward::HolsterPost]))
		MF_ExecuteForward(Weapon.Forwards[WForward::HolsterPost], NUM_FOR_EDICT(ENT(BaseWeapon->pev)));
}

static float __fastcall Weapon_GetMaxSpeed(CBasePlayerWeapon *BaseWeapon, int)
{
	if (!CUSTOM_WEAPON(BaseWeapon))
		return static_cast<float(__fastcall *)(CBasePlayerWeapon *)>(FWeapon_GetMaxSpeed[WEAPON_FID(BaseWeapon)])(BaseWeapon);

	float Speed = Weapons[WEAPON_KEY(BaseWeapon)].Speed;

	if (BaseWeapon->m_pPlayer->m_iFOV == DEFAULT_FOV)
		return Speed;

	return Speed - 40.0f;
}

static int __fastcall Weapon_ExtractAmmo(CBasePlayerWeapon *BaseWeapon, int, CBasePlayerWeapon *Original)
{
	if (!BaseWeapon->m_iDefaultAmmo)
		return NULL;

	if (CUSTOM_WEAPON(BaseWeapon))
		return Player_GiveAmmo(BaseWeapon->m_pPlayer, NULL, Ammos[BaseWeapon->m_iPrimaryAmmoType].Amount, Ammos[BaseWeapon->m_iPrimaryAmmoType].Name, -1);
	else
		return static_cast<int(__fastcall *)(CBasePlayerWeapon *, int, CBasePlayerWeapon *)>(FWeapon_ExtractAmmo[WEAPON_FID(BaseWeapon)])(BaseWeapon, NULL, Original);
}

static void __fastcall Weapon_SendWeaponAnim(CBasePlayerWeapon *BaseWeapon, int, int Anim, int SkipLocal)
{
	if (!CUSTOM_WEAPON(BaseWeapon) || !Weapons[WEAPON_KEY(BaseWeapon)].AnimR)
		static_cast<int(__fastcall *)(CBasePlayerWeapon *, int, int, int)>(FWeapon_SendWeaponAnim[WEAPON_FID(BaseWeapon)])(BaseWeapon, NULL, Anim, SkipLocal);
}

void SendWeaponAnim(CBasePlayerWeapon *BaseWeapon, int Anim)
{
	entvars_t *PlayerEntVars = BaseWeapon->m_pPlayer->pev;
	PlayerEntVars->weaponanim = Anim;
	MESSAGE_BEGIN(MSG_ONE, SVC_WEAPONANIM, NULL, ENT(PlayerEntVars));
	WRITE_BYTE(Anim);
	WRITE_BYTE(0);
	MESSAGE_END();
}

static BOOL __fastcall Player_TakeDamage(CBasePlayer *BasePlayer, int, entvars_t *Inflictor, entvars_t *Attacker, float Damage, int DamageBits)
{
	BOOL Result;
	edict_t *PlayerEdict = ENT(Attacker);
	CBasePlayerItem *BaseItem = NULL;
	CWeapon *Weapon = NULL;

	if (!InvalidEntity(PlayerEdict) && Attacker->flags & FL_CLIENT)
		BaseItem = ((DamageBits == DMG_EXPLOSION) && Inflictor->euser1) ? (CBasePlayerItem *)Inflictor->euser1->pvPrivateData : ((CBasePlayer *)PlayerEdict->pvPrivateData)->m_pActiveItem;

	if (BaseItem && CUSTOM_WEAPON(BaseItem) && ((DamageBits & DMG_BULLET) | (DamageBits & DMG_EXPLOSION)))
	{
		Weapon = &Weapons[WEAPON_KEY(BaseItem)];

		if (DamageBits == DMG_BULLET)
			Damage *= (Weapon->A2I == A2_Switch && WEAPON_INA2(BaseItem)) ? Weapon->A2V->WA2_SWITCH_DAMAGE : Weapon->Damage;
	}

	Result = static_cast<BOOL(__fastcall *)(CBasePlayer *, int, entvars_t *, entvars_t *, float, int)>(BasePlayer->IsBot() ? FPlayerBot_TakeDamage : FPlayer_TakeDamage)(BasePlayer, 0, Inflictor, Attacker, Damage, DamageBits);

	if (Weapon)
	{
		if (Weapon->Forwards[WForward::DamagePost])
			MF_ExecuteForward(Weapon->Forwards[WForward::DamagePost], NUM_FOR_EDICT(ENT(BaseItem->pev)), Damage, NUM_FOR_EDICT(ENT(BasePlayer->pev)));
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
	if (Ammos[AmmoID].Max < 0 || BasePlayer->m_rgAmmo[AmmoID] < Ammos[AmmoID].Max)
		return TRUE;

	return FALSE;
}

static void TabulateAmmo(CBasePlayer *BasePlayer)
{
	BasePlayer->ammo_buckshot = INT_MAX;
	BasePlayer->ammo_9mm = INT_MAX;
	BasePlayer->ammo_556nato = INT_MAX;
	BasePlayer->ammo_556natobox = INT_MAX;
	BasePlayer->ammo_762nato = INT_MAX;
	BasePlayer->ammo_45acp = INT_MAX;
	BasePlayer->ammo_50ae = INT_MAX;
	BasePlayer->ammo_338mag = INT_MAX;
	BasePlayer->ammo_57mm = INT_MAX;
	BasePlayer->ammo_357sig = INT_MAX;
}

int Player_GiveAmmoByID(CBasePlayer *BasePlayer, int AmmoID, int Amount)
{
	if (AmmoID < 0)
		return -1;

	if (!CanHaveAmmo(BasePlayer, AmmoID))
		return -1;

	int Add = Ammos[AmmoID].Max == -1 ? 1 : min(Amount, Ammos[AmmoID].Max - BasePlayer->m_rgAmmo[AmmoID]);

	if (Add < 1)
		return -1;

	BasePlayer->m_rgAmmo[AmmoID] += Add;
	MESSAGE_BEGIN(MSG_ONE, MESSAGE_AMMOPICKUP, NULL, ENT(BasePlayer->pev));
	WRITE_BYTE(AmmoID);
	WRITE_BYTE(Add);
	MESSAGE_END();
	EMIT_SOUND_SHORT(ENT(BasePlayer->pev), CHAN_ITEM, "items/9mmclip1.wav");
	TabulateAmmo(BasePlayer);
	return AmmoID;
}

static int __fastcall Player_GiveAmmo(CBasePlayer *BasePlayer, int, int Amount, const char *Name, int Max)
{
	if (BasePlayer->pev->flags & FL_SPECTATOR)
		return -1;

	if (!Name)
		return -1;

	int AmmoID = GetAmmoByName(Name);

	return Player_GiveAmmoByID(BasePlayer, AmmoID, Amount);
}

static void __fastcall WeaponBox_Spawn(CWeaponBox *WeaponBox, int)
{
	WeaponBox->pev->skin = INT_MAX;
	static_cast<void(__fastcall *)(CWeaponBox *WeaponBox)>(FWeaponBox_Spawn)(WeaponBox);
}

static void __fastcall TraceAttackPlayer(CBaseEntity *BaseEntity, int, entvars_t *AttackerVars, float Damage, Vector Direction, TraceResult *TResult, int DamageBits)
{
	static_cast<void(__fastcall *)(CBaseEntity *, int, entvars_t *, float, Vector, TraceResult *, int)>(FTraceAttackPlayer)(BaseEntity, 0, AttackerVars, Damage, Direction, TResult, DamageBits);

	if (DamageBits & DMG_BULLET)
		TraceAttackContinue(BaseEntity, AttackerVars, TResult);
}

static void __fastcall TraceAttack(CBaseEntity *BaseEntity, int, entvars_t *AttackerVars, float Damage, Vector Direction, TraceResult *TResult, int DamageBits)
{
	static_cast<void(__fastcall *)(CBaseEntity *, int, entvars_t *, float, Vector, TraceResult *, int)>(FTraceAttackEntity)(BaseEntity, 0, AttackerVars, Damage, Direction, TResult, DamageBits);

	if (DamageBits & DMG_BULLET)
		TraceAttackContinue(BaseEntity, AttackerVars, TResult);
}

static void TraceAttackContinue(CBaseEntity *BaseEntity, entvars_t *AttackerVars, TraceResult *TResult)
{
	if (InvalidEntity(ENT(AttackerVars)))
		return;

	CBaseEntity *BaseAttacker = (CBaseEntity *)ENT(AttackerVars)->pvPrivateData;

	if (!BaseAttacker->IsPlayer())
		return;

	CBasePlayerWeapon *BaseWeapon = (CBasePlayerWeapon *)((CBasePlayer *)BaseAttacker)->m_pActiveItem;

	if (!CUSTOM_WEAPON(BaseWeapon))
		return;

	if (!(WEAPON_FLAGS(BaseWeapon) & WFlag::NoDecal))
	{
		if (!BaseEntity->IsPlayer())
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

	if (!(WEAPON_FLAGS(BaseWeapon) & WFlag::NoSmoke) && MI_SmokePuff)
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

static void __fastcall ProjectileThink(CBaseEntity *BaseEntity, int)
{
	if (PROJECTILE_TYPE(BaseEntity) != PLIMITED_TIME)
		static_cast<void(__fastcall *)(CBaseEntity *BaseEntity)>(FProjectileThink)(BaseEntity);
	else
		MF_ExecuteForward(PROJECTILE_FORWARD(BaseEntity), NUM_FOR_EDICT(ENT(BaseEntity->pev)));
}

static void __fastcall ProjectileTouch(CBaseEntity *BaseEntity, int, CBaseEntity *Touched)
{
	if (PROJECTILE_TYPE(BaseEntity) != PLIMITED_TOUCH)
		static_cast<void(__fastcall *)(CBaseEntity *BaseEntity, int, CBaseEntity *)>(FProjectileTouch)(BaseEntity, NULL, Touched);
	else
	{
		if (BaseEntity->pev->owner != ENT(Touched->pev))
			MF_ExecuteForward(PROJECTILE_FORWARD(BaseEntity), NUM_FOR_EDICT(ENT(BaseEntity->pev)));
	}
}

static void __fastcall EffectThink(CBaseEntity *BaseEntity, int)
{
	if (EFFECT_TYPE(BaseEntity) != ELIMITED_TIME)
		static_cast<void(__fastcall *)(CBaseEntity *BaseEntity)>(FEffectThink)(BaseEntity);
	else
	{
		float NextFrame = BaseEntity->pev->frame + BaseEntity->pev->framerate * (gpGlobals->time - EFFECT_LAST_TIME(BaseEntity));
		BaseEntity->pev->frame = fmod(NextFrame, EFFECT_MAX_FRAMES(BaseEntity));

		if (EFFECT_EXPIRE_TIME(BaseEntity) && gpGlobals->time >= EFFECT_EXPIRE_TIME(BaseEntity))
			MF_ExecuteForward(PROJECTILE_FORWARD(BaseEntity), NUM_FOR_EDICT(ENT(BaseEntity->pev)));
		else if (NextFrame >= EFFECT_MAX_FRAMES(BaseEntity))
			MF_ExecuteForward(PROJECTILE_FORWARD(BaseEntity), NUM_FOR_EDICT(ENT(BaseEntity->pev)));

		BaseEntity->pev->nextthink = gpGlobals->time + 0.01f;
		EFFECT_LAST_TIME(BaseEntity) = gpGlobals->time;
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

inline int *GetPlayerAmmo(CBasePlayer *BasePlayer, int AmmoID)
{
	return &BasePlayer->m_rgAmmo[AmmoID];
}

void GiveWeapon(edict_t *PlayerEdict, int Index)
{
	if (IS_USER_DEAD(PlayerEdict))
		return;

	CWeapon &Weapon = Weapons[Index];
	edict_t *WeaponEdict = CREATE_NAMED_ENTITY(MAKE_STRING(WEAPON_CLASS[WEAPON_TYPE_ID[Weapon.Type]]));
	WeaponEdict->v.spawnflags |= SF_NORESPAWN;

	CBasePlayer *BasePlayer = static_cast<CBasePlayer *>(PlayerEdict->pvPrivateData);
	CBasePlayerWeapon *BaseWeapon = static_cast<CBasePlayerWeapon *>(WeaponEdict->pvPrivateData);
	CBasePlayerItem *BaseItem = BasePlayer->m_rgpPlayerItems[WEAPON_SLOT[WEAPON_TYPE_ID[Weapon.Type]]];

	CUSTOM_WEAPON(BaseWeapon) = TRUE;
	BaseWeapon->m_iPrimaryAmmoType = Weapon.AmmoID;

	if (Weapon.Flags & WFlag::SoloClip)
		BasePlayer->m_rgAmmo[BaseWeapon->m_iPrimaryAmmoType] = Ammos[Weapon.AmmoID].Max;

	if (BaseItem)
		UTIL_FakeClientCommand(PlayerEdict, "drop", WEAPON_CLASS[BaseItem->m_iId]);

	WEAPON_KEY(BaseWeapon) = WEAPON_KEY_EX(WeaponEdict) = Index;
	WEAPON_CLIP(BaseWeapon) = (Weapon.Flags & WFlag::SoloClip) ? -1 : Weapon.Clip;
	WEAPON_FLAGS(BaseWeapon) = Weapon.Flags;
	BaseWeapon->ResetEmptySound();

	if (Weapon.A2I)
	{
		switch (WEAPON_A2(BaseWeapon) = Weapon.A2I)
		{
			case A2_Burst: WEAPON_A2_OFFSET(BaseWeapon) = Weapon.A2V->WA2_BURST_VALUE; break;
			case A2_MultiShot: WEAPON_A2_OFFSET(BaseWeapon) = Weapon.A2V->WA2_MULTISHOT_VALUE; break;
		}
	}

	MDLL_Spawn(WeaponEdict);
	BaseWeapon->m_iDefaultAmmo = NULL;
	MDLL_Touch(WeaponEdict, PlayerEdict);

	if (Weapon.Forwards[WForward::SpawnPost])
		MF_ExecuteForward(Weapon.Forwards[WForward::SpawnPost], NUM_FOR_EDICT(WeaponEdict));
}

void PlaybackEvent(int Flags, const edict_t *Invoker, unsigned short EI, float Delay, float *Origin, float *Angles, float F1, float F2, int I1, int I2, int B1, int B2)
{
	SET_META_RESULT(MRES_IGNORED);

	if (!Invoker)
		return;

	CBasePlayerWeapon *BaseWeapon = ((CBasePlayerWeapon *)((CBasePlayer *)(Invoker->pvPrivateData))->m_pActiveItem);

	if (!BaseWeapon || !CUSTOM_WEAPON(BaseWeapon))
		return;

	SET_META_RESULT(MRES_SUPERCEDE);
	PLAYBACK_EVENT_FULL(Flags | FEV_HOSTONLY, Invoker, EI, Delay, Origin, Angles, F1, F2, I1, I2, B1, B2);
}

void SetModel(edict_t *Edict, const char *Model)
{
	if (Edict->v.skin != INT_MAX)
		RETURN_META(MRES_IGNORED);

	CWeaponBox *WeaponBox = (CWeaponBox *)Edict->pvPrivateData;
	CBasePlayerItem *BaseItem = WeaponBox->m_rgpPlayerItems[1];

	if (!BaseItem)
		RETURN_META(MRES_IGNORED);

	BaseItem = WeaponBox->m_rgpPlayerItems[2];

	if (!BaseItem)
		RETURN_META(MRES_IGNORED);

	if (InvalidEntity(ENT(BaseItem->pev)) || !CUSTOM_WEAPON(BaseItem))
		RETURN_META(MRES_IGNORED);

	CWeapon &Weapon = Weapons[WEAPON_KEY(BaseItem)];
	SET_MODEL(Edict, Weapon.WModel);

	if (Weapon.Forwards[WForward::DropPost])
		MF_ExecuteForward(Weapon.Forwards[WForward::DropPost], NUM_FOR_EDICT(ENT(BaseItem->pev)), NUM_FOR_EDICT(Edict));

	RETURN_META(MRES_SUPERCEDE);
}

void UpdateClientData_Post(const edict_s *PlayerEdict, int SendWeapons, clientdata_s *CD)
{
	CBasePlayerItem *BaseWeapon = (((CBasePlayer *)PlayerEdict->pvPrivateData)->m_pActiveItem);

	if (BaseWeapon && CUSTOM_WEAPON(BaseWeapon))
		CD->m_flNextAttack = gpGlobals->time + 0.001f;

	RETURN_META(MRES_IGNORED);
}

void BuyAmmo(edict_t *PlayerEdict, int Slot)
{
	CBasePlayer *BasePlayer = (CBasePlayer *)PlayerEdict->pvPrivateData;
	CBasePlayerItem *BaseItem = BasePlayer->m_rgpPlayerItems[Slot];

	if (!BaseItem)
		return;

	int *PlayerMoney = &BasePlayer->m_iAccount;
	int AmmoID = ((CBasePlayerWeapon *)BaseItem)->m_iPrimaryAmmoType;

	if (!(BasePlayer->m_signals.GetState() & SIGNAL_BUY))
		return;

	if (CUSTOM_WEAPON(BaseItem) && WEAPON_FLAGS(BaseItem) & WFlag::SoloClip)
		return;

	CAmmo *Ammo = &Ammos[AmmoID];

	if (BasePlayer->m_rgAmmo[AmmoID] >= Ammo->Max)
		return;

	if (Ammo->Cost > 0 && *PlayerMoney < Ammo->Cost)
	{
		ClientPrint(PlayerEdict, HUD_PRINTCENTER, "#Not_Enough_Money");
		goto Flash;
	}

	Player_GiveAmmoByID(BasePlayer, AmmoID, Ammo->Amount);
	MESSAGE_BEGIN(MSG_ONE, MESSAGE_MONEY, NULL, PlayerEdict);
	WRITE_LONG(*PlayerMoney -= Ammo->Cost);
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
		for (int Index = NULL; Index < WeaponCount; Index++)
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

void StatusIcon(edict_t *PlayerEdict, BOOL Status, const char *Icon, int R, int G, int B)
{
	MESSAGE_BEGIN(MSG_ONE, MESSAGE_STATUSICON, NULL, PlayerEdict);
	WRITE_BYTE(Status);
	WRITE_STRING(Icon);
	WRITE_BYTE(R);
	WRITE_BYTE(G);
	WRITE_BYTE(B);
	MESSAGE_END();
}

void StatusIconNumber(edict_t *PlayerEdict, BOOL Status, char Number)
{
	static char NumString[] = "number_0";
	NumString[7] = Number;
	MESSAGE_BEGIN(MSG_ONE, MESSAGE_STATUSICON, NULL, PlayerEdict);
	WRITE_BYTE(Status);
	WRITE_STRING(NumString);
	WRITE_BYTE(0);
	WRITE_BYTE(175);
	WRITE_BYTE(125);
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

void PlayerKnockback(edict_t *VictimEdict, Vector &Origin, float Knockback)
{
	Vector VDistance = (VictimEdict->v.origin - Origin);
	float FLength = VDistance.Length();
	VDistance = (VDistance / FLength) * Knockback;
	VDistance[2] = Knockback * RANDOM_FLOAT(0.9f, 1.1f);
	VictimEdict->v.velocity = VDistance;
}

static void PerformDamage(edict_t *PlayerEdict, float Radius, float Damage, CBasePlayerWeapon *BaseWeapon, BOOL Accurate, float Knockback)
{
	BOOL Result;
	Vector MyOrigin = PlayerEdict->v.origin + PlayerEdict->v.view_ofs, Origin;
	edict_t *TargetEdict = NULL;
	TraceResult TResult;
	CBaseEntity *BaseEntity;
	entvars_t *PlayerEntVars = &PlayerEdict->v;

	while ((TargetEdict = FIND_ENTITY_IN_SPHERE(TargetEdict, PlayerEdict->v.origin, Radius)) != SVGame_Edicts)
	{
		if (!TargetEdict->pvPrivateData || TargetEdict == PlayerEdict)
			continue;

		if (!TargetEdict->v.takedamage)
			continue;

		BaseEntity = (CBaseEntity *)TargetEdict->pvPrivateData;
		Origin = TargetEdict->v.origin;

		if (BaseEntity->IsBSPModel())
			Origin = Origin + (TargetEdict->v.mins + TargetEdict->v.maxs) * 0.5f;

		if (!InViewCone(PlayerEdict, Origin, Accurate))
			continue;

		if (!(WEAPON_FLAGS(BaseWeapon) & WFlag::KnifeAttack_Penetration))
		{
			TRACE_LINE(MyOrigin, Origin, 0, PlayerEdict, &TResult);

			if (TResult.pHit != TargetEdict)
				continue;
		}

		Result = BaseEntity->TakeDamage(PlayerEntVars, PlayerEntVars, Damage, DMG_SLASH);

		if (Result && BaseEntity->IsPlayer() && (WEAPON_FLAGS(BaseWeapon) & WFlag::KnifeAttack_Knockback))
			PlayerKnockback(TargetEdict, MyOrigin, Knockback);
	}

}

/* ===================================================== */
/* =================== ATTACK 2 ======================== */
/* ===================================================== */

static void Attack2_Zoom(CBasePlayer *BasePlayer, CBasePlayerWeapon *BaseWeapon, int Type)
{
	int FOV = BasePlayer->m_iFOV;

	switch (Type)
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

	BasePlayer->m_iFOV = FOV;
	WEAPON_INA2(BaseWeapon) = !WEAPON_INA2(BaseWeapon);
	CLIENT_COMMAND(ENT(BasePlayer->pev), "spk weapons/zoom\n");
}

static void Attack2_Switch(CBasePlayer *BasePlayer, CBasePlayerWeapon *BaseWeapon, CWeapon &Weapon)
{
	edict_t *PlayerEdict = ENT(BasePlayer->pev);

	switch (WEAPON_INA2(BaseWeapon))
	{
		case FALSE:
		{
			SendWeaponAnim(BaseWeapon, Weapon.A2V->WA2_SWITCH_ANIM_A);
			BaseWeapon->m_flNextReload = BaseWeapon->m_flNextPrimaryAttack = BaseWeapon->m_flNextSecondaryAttack = BaseWeapon->m_flTimeWeaponIdle = Weapon.A2V->WA2_SWITCH_ANIM_A_DURATION;

			if (!(Weapon.Flags & SwitchMode_NoText))
				CLIENT_PRINT(PlayerEdict, print_center, "Switch to Mode B");

			break;
		}
		case TRUE:
		{
			SendWeaponAnim(BaseWeapon, Weapon.A2V->WA2_SWITCH_ANIM_B);
			BaseWeapon->m_flNextReload = BaseWeapon->m_flNextPrimaryAttack = BaseWeapon->m_flNextSecondaryAttack = BaseWeapon->m_flTimeWeaponIdle = Weapon.A2V->WA2_SWITCH_ANIM_B_DURATION;

			if (!(Weapon.Flags & SwitchMode_NoText))
				CLIENT_PRINT(PlayerEdict, print_center, "Switch to Mode B");

			break;
		}
	}

	if (Weapon.Flags & SwitchMode_BarTime)
		BarTime(PlayerEdict, *(int *)&BaseWeapon->m_flNextPrimaryAttack);

	WEAPON_INA2_DELAY(BaseWeapon) = TRUE;
}

static void Attack2_Burst(CBasePlayer *BasePlayer, CBasePlayerWeapon *BaseWeapon, CWeapon &Weapon)
{
	entvars_t *PlayerEntVars = BasePlayer->pev;

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

			if ((1 << BaseWeapon->m_iId) & SECONDARY_WEAPONS_BIT_SUM)
				ClientPrint(ENT(PlayerEntVars), HUD_PRINTCENTER, "#Switch_To_SemiAuto");
			else
				ClientPrint(ENT(PlayerEntVars), HUD_PRINTCENTER, "#Switch_To_FullAuto");
			break;
		}
	}

	BaseWeapon->m_flNextSecondaryAttack = 0.2f;
}

static void Attack2_MultiShot(CBasePlayerWeapon *BaseWeapon, CWeapon &Weapon)
{
	if (BaseWeapon->m_flNextPrimaryAttack > 0.0f)
		return;

	WEAPON_INA2(BaseWeapon) = TRUE;

	for (int Index = 0; Index < WEAPON_A2_OFFSET(BaseWeapon); Index++)
	{
		if (BaseWeapon->m_iClip > 0)
			BaseWeapon->PrimaryAttack();
	}

	BaseWeapon->m_iShotsFired = TRUE;
	BaseWeapon->m_flNextSecondaryAttack = Weapon.Delay;
	WEAPON_INA2(BaseWeapon) = FALSE;
}

static void Attack2_KnifeAttack(CBasePlayer *BasePlayer, CBasePlayerWeapon *BaseWeapon, CWeapon &Weapon)
{
	BaseWeapon->m_flNextSecondaryAttack = Weapon.A2V->WA2_KNIFEATTACK_DELAY;
	BaseWeapon->m_flTimeWeaponIdle = BaseWeapon->m_flNextPrimaryAttack = Weapon.A2V->WA2_KNIFEATTACK_DURATION;
	SendWeaponAnim(BaseWeapon, Weapon.A2V->WA2_KNIFEATTACK_ANIMATION);

	WEAPON_INA2_DELAY(BaseWeapon) = TRUE;
	EMIT_SOUND_SHORT(ENT(BaseWeapon->pev), CHAN_VOICE, Weapon.A2V->WA2_KNIFEATTACK_SOUND);
}

static void Attack2_KnifeAttackPerform(CBasePlayerWeapon *BaseWeapon)
{
	WEAPON_INA2_DELAY(BaseWeapon) = FALSE;
	BaseWeapon->m_flNextSecondaryAttack = BaseWeapon->m_flNextPrimaryAttack;

	CBasePlayer *BasePlayer = BaseWeapon->m_pPlayer;
	CWeapon &Weapon = Weapons[WEAPON_KEY(BaseWeapon)];

	if (Weapon.Flags & WFlag::KnifeAttack_ScreenShake)
		ScreenShake(ENT(BasePlayer->pev), 4, 4, 4);

	PerformDamage(ENT(BasePlayer->pev), Weapon.A2V->WA2_KNIFEATTACK_RADIUS, (float)RANDOM_LONG(Weapon.A2V->WA2_KNIFEATTACK_DAMAGE_MIN, Weapon.A2V->WA2_KNIFEATTACK_DAMAGE_MAX), BaseWeapon, Weapon.Flags & WFlag::KnifeAttack_Accurate, Weapon.A2V->WA2_KNIFEATTACK_KNOCKBACK);
}

static void Attack2_InstaSwitch(edict_t *PlayerEdict, CBasePlayerWeapon *BaseWeapon, CWeapon &Weapon)
{
	switch (WEAPON_INA2(BaseWeapon))
	{
		case FALSE:
		{
			WEAPON_INA2(BaseWeapon) = TRUE;
			ClientPrint(PlayerEdict, HUD_PRINTCENTER, Weapon.A2V->WA2_INSTASWITCH_NAME);
			break;
		}
		case TRUE:
		{
			WEAPON_INA2(BaseWeapon) = FALSE;
			ClientPrint(PlayerEdict, HUD_PRINTCENTER, Weapon.A2V->WA2_INSTASWITCH_NAME2);
			break;
		}
	}

	BaseWeapon->m_flNextSecondaryAttack = 0.2f;
}

static void Attack2_ZoomCustom(CBasePlayer *BasePlayer, CBasePlayerWeapon *BaseWeapon, int FOV)
{
	if (WEAPON_INA2(BaseWeapon))
		BasePlayer->m_iFOV = 90;
	else
		BasePlayer->m_iFOV = FOV;

	WEAPON_INA2(BaseWeapon) = !WEAPON_INA2(BaseWeapon);
	CLIENT_COMMAND(ENT(BasePlayer->pev), "spk weapons/zoom\n");
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
	entvars_t *ProjectileEntVars = BaseEntity->pev;
	entvars_t *LauncherEntVars = &LauncherEdict->v;
	CProjectile *Projectile = &Projectiles[ProjectileID];
	Vector Velocity = gpGlobals->v_forward * Projectile->Speed;

	BaseEntity->Spawn();
	BaseEntity->m_iTeam = GetEntityTeam(LauncherEdict);
	ProjectileEntVars->movetype = MOVETYPE_PUSHSTEP;
	ProjectileEntVars->solid = SOLID_TRIGGER;
	SET_MODEL(ProjectileEdict, Projectile->Model);
	//SET_SIZE(ProjectileEdict, ZVector, ZVector);
	SET_ORIGIN(ProjectileEdict, LauncherEntVars->origin + LauncherEntVars->view_ofs + gpGlobals->v_forward * 16);
	ProjectileEntVars->gravity = Projectile->Gravity;
	ProjectileEntVars->velocity = Velocity;
	ProjectileEntVars->angles = VectorToAngles(Velocity);
	ProjectileEntVars->owner = LauncherEdict;
	ProjectileEntVars->euser1 = ENT(((CBasePlayer *)LauncherEdict->pvPrivateData)->m_pActiveItem->pev);
	ProjectileEntVars->nextthink = gpGlobals->time + Projectile->Duration;

	PROJECTILE_TYPE(BaseEntity) = PLIMITED_TIME;
	PROJECTILE_FORWARD(BaseEntity) = Projectile->Forward;
	return NUM_FOR_EDICT(ProjectileEdict);
}

cell ShootProjectileContact(edict_t *LauncherEdict, int ProjectileID)
{
	edict_t *ProjectileEdict = CREATE_NAMED_ENTITY(MAKE_STRING(PROJECTILE_CLASS));
	CGrenade *BaseEntity = ((CGrenade *)ProjectileEdict->pvPrivateData);
	entvars_t *ProjectileEntVars = BaseEntity->pev;
	entvars_t *LauncherEntVars = &LauncherEdict->v;
	CProjectile *Projectile = &Projectiles[ProjectileID];
	Vector Origin = LauncherEdict->v.origin + LauncherEdict->v.view_ofs + gpGlobals->v_forward * 16;
	Vector Angles = LauncherEdict->v.angles;
	Angles[2] += RANDOM_FLOAT(10.0f, 16.0f) * 20.0f;

	BaseEntity->Spawn();
	BaseEntity->m_iTeam = GetEntityTeam(LauncherEdict);
	ProjectileEntVars->movetype = MOVETYPE_PUSHSTEP;
	ProjectileEntVars->solid = SOLID_TRIGGER;
	SET_MODEL(ProjectileEdict, Projectile->Model);
	//SET_SIZE(ProjectileEdict, Vector(0, 0, 0), Vector(0, 0, 0));
	SET_ORIGIN(ProjectileEdict, LauncherEntVars->origin + LauncherEntVars->view_ofs + gpGlobals->v_forward * 16);
	ProjectileEntVars->gravity = Projectile->Gravity;
	ProjectileEntVars->velocity = gpGlobals->v_forward * Projectile->Speed;
	ProjectileEntVars->angles = Angles;
	ProjectileEntVars->owner = LauncherEdict;
	ProjectileEntVars->euser1 = ENT(((CBasePlayer *)LauncherEdict->pvPrivateData)->m_pActiveItem->pev);

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
	EffectEntVars->nextthink = EFFECT_LAST_TIME(BaseEntity) = gpGlobals->time;
	EffectEntVars->velocity = gpGlobals->v_forward * Effect->Speed;

	//SET_SIZE(EffectEdict, ZVector, ZVector);
	EFFECT_TYPE(BaseEntity) = ELIMITED_TIME;
	EFFECT_MAX_FRAMES(BaseEntity) = (float)MODEL_FRAMES(EffectEntVars->modelindex) - 1;
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

	if (SpecialBot_VTable[EO_TakeDamage] == FPlayer_TakeDamage)
		FPlayerBot_TakeDamage = FPlayer_TakeDamage;
	else
		HookEntityFWByVTable(SpecialBot_VTable, EO_TakeDamage, Player_TakeDamage, FPlayerBot_TakeDamage);
}

void *FWeapon_SpawnFuncs[MAX_WEAPON_TYPES] = { Weapon_SpawnPistol, Weapon_SpawnShotgun, Weapon_SpawnRifle, Weapon_SpawnSniper };
void *FWeapon_ReloadFuncs[] = { Weapon_Reload, Weapon_ReloadShotgun, Weapon_Reload, Weapon_Reload };
void *FWeapon_IdleFuncs[] = { Weapon_Idle, Weapon_IdleShotgun, Weapon_Idle, Weapon_Idle };

const int HOOK_OFFSETS[] = { EO_Spawn, EO_Item_AddToPlayer, EO_Item_Deploy, EO_Weapon_PrimaryAttack, EO_Weapon_SecondaryAttack, EO_Weapon_Reload, EO_Item_PostFrame, EO_Weapon_Idle, EO_Item_Holster, EO_Item_GetMaxSpeed, EO_Weapon_ExtractAmmo, EO_Weapon_SendWeaponAnim };
const void *HOOK_FUNCS[] = { FWeapon_SpawnFuncs, Weapon_AddToPlayer, Weapon_Deploy, Weapon_PrimaryAttack, Weapon_SecondaryAttack, FWeapon_ReloadFuncs, Weapon_PostFrame, FWeapon_IdleFuncs, Weapon_Holster, Weapon_GetMaxSpeed, Weapon_ExtractAmmo, Weapon_SendWeaponAnim };
const int HOOK_ARRAY_FUNCS = (1 << 0) | (1 << 5) | (1 << 7);
const void *HOOK_FORWARDS[] = { FWeapon_Spawn, FWeapon_AddToPlayer, FWeapon_Deploy, FWeapon_PrimaryAttack, FWeapon_SecondaryAttack, FWeapon_Reload, FWeapon_PostFrame, FWeapon_Idle, FWeapon_Holster, FWeapon_GetMaxSpeed, FWeapon_ExtractAmmo, FWeapon_SendWeaponAnim };

BOOL DispatchSpawn(edict_t *Entity)
{
	SET_META_RESULT(MRES_IGNORED);

	if (!PrecachedWeapons)
	{
		PrecachedWeapons = TRUE;
		LoadWeapons();
	}

	if (EntityHooked)
		return NULL;

	SET_META_RESULT(MRES_IGNORED);

	const char *WeaponName;
	void *WeaponFunction;

	for (int Index = 0; Index < MAX_WEAPON_TYPES; Index++)
	{
		WeaponName = WEAPON_CLASS[WEAPON_TYPE_ID[Index]];

		for (int FIndex = 0; FIndex < MAX_HOOKS; FIndex++)
		{
			if ((1 << FIndex) & HOOK_ARRAY_FUNCS)
				WeaponFunction = ((void **)HOOK_FUNCS[FIndex])[Index];
			else
				WeaponFunction = (void *)HOOK_FUNCS[FIndex];

			HookEntityFW(WeaponName, HOOK_OFFSETS[FIndex], WeaponFunction, ((void **)HOOK_FORWARDS[FIndex])[Index]);
		}
	}

	HookEntityFW(PLAYER_CLASS, EO_TakeDamage, Player_TakeDamage, FPlayer_TakeDamage);
	HookEntityFW(PLAYER_CLASS, EO_GiveAmmo, Player_GiveAmmo, FPlayer_GiveAmmo);
	HookEntityFW("weaponbox", EO_Spawn, WeaponBox_Spawn, FWeaponBox_Spawn);
	HookEntityFW("worldspawn", EO_TraceAttack, TraceAttack, FTraceAttackEntity);
	HookEntityFW("func_wall", EO_TraceAttack, TraceAttack, FTraceAttackEntity);
	HookEntityFW("func_tank", EO_TraceAttack, TraceAttack, FTraceAttackEntity);
	HookEntityFW("func_breakable", EO_TraceAttack, TraceAttack, FTraceAttackEntity);
	HookEntityFW("func_door", EO_TraceAttack, TraceAttack, FTraceAttackEntity);
	HookEntityFW("func_door_rotating", EO_TraceAttack, TraceAttack, FTraceAttackEntity);
	HookEntityFW(PLAYER_CLASS, EO_TraceAttack, TraceAttackPlayer, FTraceAttackPlayer);
	HookEntityFW(PROJECTILE_CLASS, EO_Think, ProjectileThink, FProjectileThink);
	HookEntityFW(PROJECTILE_CLASS, EO_Touch, ProjectileTouch, FProjectileTouch);
	HookEntityFW(EFFECT_CLASS, EO_Think, EffectThink, FEffectThink);

	FPlayerKilled = GetEntityFW(PLAYER_CLASS, EO_Killed);
	EntityHooked = TRUE;
	return NULL;
}

void UpdateAmmoList()
{
	for (int Index = NULL; Index < AMMO_MAX_TYPES; Index++)
		Ammos.Append(DEFAULT_AMMOS[Index]);
}

BOOL DispatchSpawn_Post(edict_s *Entity)
{
	SET_META_RESULT(MRES_IGNORED);

	if (Initialized)
		return NULL;

	PrecacheModule();

	if (!Ammos.Length)
		UpdateAmmoList();

	Initialized = TRUE;
	return NULL;
}

void ServerActivate(edict_t *Worldspawn, int MaxEdicts, int MaxPlayers)
{
	SET_META_RESULT(MRES_IGNORED);
	CanPrecache = FALSE;
	SVGame_Edicts = Worldspawn;
}

void ServerDeactivate_Post(void)
{
	Initialized = FALSE;
	PrecachedWeapons = FALSE;
	CanPrecache = TRUE;
	ClearWeapons = TRUE;
	AmmoCount = AMMO_MAX_TYPES;
	Ammos.Clear();
	Projectiles.Clear();
	RETURN_META(MRES_IGNORED);
}

void OnPluginsUnloaded(void)
{
	for (int Index = 0; Index < WeaponCount; Index++)
	{
		CWeapon &Weapon = Weapons[Index];

		if (Weapon.A2V)
			delete[] Weapon.A2V;
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

		if (CUSTOM_WEAPON(BaseEntity) != TRUE)
			continue;

		const char *ClassName = STRING(BaseEntity->pev->classname);

		if (ClassName[0] != 'w' || ClassName[6] != '_')
			continue;

		CUSTOM_WEAPON(BaseEntity) = 0;
	}
}