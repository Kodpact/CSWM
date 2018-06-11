/* CSWM Incude File */

#if defined _cswm_included
	#endinput
#endif
#define _cswm_included

enum _:
{
	CSWM_IGNORED,
	CSWM_HANDLED,
}

enum WAmmoType
{
	WAmmoType_None,
	WAmmoType_338Magnum,
	WAmmoType_762Nato,
	WAmmoType_556NatoBox,
	WAmmoType_556Nato,
	WAmmoType_BuckShot,
	WAmmoType_45ACP,
	WAmmoType_57MM,
	WAmmoType_50AE,
	WAmmoType_357SIG,
	WAmmoType_9MM,
	WAmmoType_FLASHBANG,
	WAmmoType_HEGRENADE,
	WAmmoType_SMOKEGRENADE,
	WAmmoType_C4,
};

enum WType
{
	Pistol,
	ShotGun,
	Rifle,
	Sniper,
}

enum
{
	Zoom_Rifle,
	Zoom_SniperF,
	Zoom_SniperS,
	Zoom_SniperB,
};

enum WAttack2
{
	WA2_None,
	WA2_Zoom,
	WA2_Switch,
	WA2_Burst,
	WA2_AutoPistol,
	WA2_MultiShot,
	WA2_KnifeAttack,
	WA2_RadiusDamage,
	WA2_Charge,
	WA2_InstaSwitch,
};

enum WTR
{
	TR_None,
	TR_LaserBeam
}

enum WForward:
{
	WForward_Purchase,
	WForward_DeployPost,
	WForward_PrimaryAttackPre,
	WForward_PrimaryAttackPost,
	WForward_SecondaryAttackPre,
	WForward_SecondaryAttackPost,
	WForward_ReloadPre,
	WForward_ReloadPost,
	WForward_HolsterPost,
	WForward_DropPost,
};

enum WData
{
	WData_IsCustom,
	WData_Attack2,
	WData_Burst,
	WData_CurBurst,
	WData_FID,
	WData_Flags,
	WData_Icon,
	WData_InAttack2,
	WData_InBurst,
	WData_InKnifeAttack,
	WData_Key,
	WData_Owner,
	WData_LastAttackInAttack2,
	WData_InSwitch,
};

#define BIT(%1) 1 << %1

enum WFlag
{
	NoHUD = BIT(0),
	AutoReload = BIT(1),
	SoloClip = BIT(2),
	NoDecal = BIT(3),
	NoSmoke = BIT(3),
	ShotGunCustomReloadSound = BIT(4),
	CustomPrimaryAttack = BIT(5),
	AutoSniper = BIT(6),
CustomIdleAnim = BIT(7),

	SwitchMode_BarTime = BIT(10),
	SwitchMode_NoText = BIT(11),
	KnifeAttack_ScreenShake = BIT(10),
	KnifeAttack_Penetration = BIT(11),
	KnifeAttack_Accurate = BIT(12),
};

/**
 * @param Model[]	Weapon Model Name (Recommended LowerCase Letters)
 * @param Type		Weapon Type ID (See WType Enum)
 * @param Name		Weapon Name
 *
 * @return			Registered Weapon ID in Weapons Array
 */
native CreateWeapon(Model[], WType:Type, Name[]);

/**
 * @param Cost		Cost of an ammo ($)
 * @param Ammount	Give Ammount
 * @param Max		Max Ammo of this Type
 *
 * @return			Registered Ammo ID in Ammos Array
 */
native WAmmoType:CreateAmmo(Cost, Ammount, Max);

/**
 * @param Weapon	Weapon ID from Weapons Array
 * @param Sound[]	Sound Name (!DONT INCLUDE SOUNDS FOLDER)
 * 
 * @noreturn
 */
native BuildWeaponFireSound(Weapon, Sound[]);

/**
 * @param Weapon	Weapon ID from Weapons Array
 * @param Clip		Clip Size of Weapon
 * @param AmmoID	Ammo ID from Ammos Array
 * 
 * @noreturn
 */
native BuildWeaponAmmunition(Weapon, Clip, WAmmoType:AmmoID);


/**
 * @param Weapon 	Weapon ID from Weapons Array
 * @param Anim		Animation ID
 * @param Duration	Animation Duration
 *
 * @noreturn
 */
native BuildWeaponDeploy(Weapon, Anim, Float:Duration);

/**
 * @param Weapon	Weapon ID from Weapons Array
 * @param Anim[3]	Animation ID Pack (!EXAMPLE: { 1, 2, 3 })
 * @param Duration	Animation Duration (!CHOOSE BIGGEST)
 * @param Delay		Time Before Next Attack
 * @param Damage	Damage Multiplier (WType * Damage) (WType = P228/XM1014/AK47)
 * @param Recoil	Recoil Multiplier
 * 
 * @noreturn
 */
native BuildWeaponShoot(Weapon, Anim[3], Float:Duration, Float:Delay, Float:Damage, Float:Recoil);

/**
 * @param Weapon	Weapon ID from Weapons Array
 * @param Anim		Animation ID
 * @param Duration	Animation Duration
 * 
 * @noreturn
 */
native BuildWeaponReload(Weapon, Anim, Float:Duration);

/**
 * @param Weapon		Weapon ID from Weapons Array
 * @param Duration		Reload Time (ShotGun Style)
 * @Param ReloadStyle	false = XM1014, true = M3
 * 
 * @noreturn
 */
native BuildWeaponReload2(Weapon, Float:Duration, ReloadStyle);

/**
 * @param Weapon	Weapon ID from Weapons Array
 * @param Flags		Flags BitSum (See WFlag Enum) (!EXAMPLE: NoHUD | AutoReload)
 * 
 * @noreturn
 */
native BuildWeaponFlags(Weapon, WFlag:Flags);

/**
 * @param Weapon	Weapon ID from Weapons Array
 * @param Name[]	Icon Name from 'hud.txt'
 * @param R			R Color Ammount (RGB Type)
 * @param G			G Color Ammount (RGB Type)
 * @param B			B Color Ammount (RGB Type)
 * @param any...	(Name[], R, G, B) Params for some Attack 2 Modes
 * 
 * @noreturn
 */
native BuildWeaponIcon(Weapon, Name[], R, G, B, any:...);

/**
 * @param Weapon	Weapon ID from Weapons Array
 * @param Type		Attack 2 Type (See WAttack2 Enum)
 * @param any...	Parameters For Specified Attack 2 Mode
 *
 * @noreturn
 */
native BuildWeaponAttack2(Weapon, WAttack2:Type, any:...);

/**
 * @param Weapon	Weapon ID from Weapons Array
 * @param Type		Trace Attack Type (See WTR Enum)
 * @noreturn
 */
native BuildWeaponTraceAttack(Weapon, WTR:Type, any:...);

/**
 * @param Model		Model To Be Precached
 * @param any...	Other Models To Be Precached
 * 
 * @noreturn
 */
native PrecacheWeaponModel(Model[], any:...);

/**
 * @param Sound		Sound To Be Precached
 * @param any...	Other Sound To Be Precached
 * 
 * @noreturn
 */
native PrecacheWeaponSound(Sound[], any:...);

/**
 * @param Generic		Generic Files To Be Precached
 * @param any...		Other Generic Files To Be Precached
 * 
 * @noreturn
 */
native PrecacheWeaponGeneric(Generic[], any:...);

/**
 * @param Weapon	Weapon ID from Weapons Array
 * @param Forward	Weapon Forward ID (See WForward Enum)
 * @param CallBack	CallBack Function
 *
 * @noreturn
 */
native RegisterWeaponForward(Weapon, WForward:Forward, CallBack[]);

/**
 * @param WeaponEntIndex	Entity Index of Weapon
 * @param Data				Data ID to Get (See WData Enum)
 *
 * @noreturn
 */
native GetWeaponData(WeaponEntIndex, WData:Data);

/**
 * @param WeaponEntIndex	Entity Index of Weapon
 * @param Data				Data ID to Get (See WData Enum)
 *
 * @noreturn
 */
native SetWeaponData(WeaponEntIndex, WData:Data, Value);


/**
 * @param PlayerID		Client Index
 * @param AmmoID		Ammo ID from Ammos Array
 *
 * @noreturn
 */
native GiveAmmo(PlayerID, AmmoID);

/**
 * @param AmmoID		Ammo ID from Ammos Array
 * @param Name[]		Name for Ammo ID
 *
 * @noreturn
 */
native SetAmmoName(WAmmoType:Ammo, Name[]);

/**
 * @param Name[]		Name for Ammo ID
 *
 * @return				(Ammo ID in Ammos Array) or NULL
 */
native FindAmmoByName(Name[]);

/**
 * @param Model[]		Model Name of ProjectileID
 * @param Gravity		Gravity Ammount for Projectile
 * @param CallBack		Function to Call (Happens When Hits Something (!EXAMPLE: Ground))
 *
 * @return				Projectile ID in Projectiles Array
 */
native CreateProjectile(const Model[], Float:Gravity, Float:Speed, CallBack[]);

/**
 * @param Launcher		Client Index who Launches Projectile
 * @param ProjectileID	Projectile ID from Projectiles Array
 *
 * @noreturn
 */
native ShootProjectile(Launcher, ProjectileID);

/**
 * @param PlayerID		Player Index
 * @param WeaponID		Weapon ID from Weapons Array
 *
 * @return				True or False
 */
native GiveWeaponByID(PlayerID, WeaponID);

/**
 * @param Name[]		Weapon Name for Search
 *
 * @return				(Weapon ID from Weapons Array) or NULL
 */
native FindWeaponByName(Name[]);

/**
 * @param PlayerID		Player Index
 * @param Name[]		Weapon Name to Give
 *
 * @return				True or False
 */
native GiveWeaponByName(PlayerID, Name[]);

/**
 * @param WeaponEntIndex	Weapon Entity Index
 * @param AnimationID		Animation ID
 *
 * @noreturn
 */
native SendWeaponAnim(WeaponEntIndex, AnimationID);

/**
 *
 * @return					Weapon Count
 */
native GetWeaponCount();

/**
 * @param Index				Weapon Index
 * @param OutPut			String Destination
 * @param Len				Length of String
 *
 * @noreturn
 */
native GetWeaponName(Index, OutPut[], Len);

/**
 * @param AddOn				AddOn Path Name
 *
 * @noreturn
 */
native SetWeaponPathAddOn(AddOn[]);


/**
 * @param Index				Weapon Index
 *
 * @return					Flags BitSum
 */
native GetWeaponFlags(Index);

/**
 * @param Index				Weapon Index
 *
 * @return					Flags BitSum
 */
native RadiusDamage2(Origin[3], Float:Radius, Float:Damage, Caster);


native CanPrimaryAttack(EntityID);
native SetNextAttack(EntityID, Float:Value);
native GiveItem(PlayerID, const Name[]);
native SetCustomIdleAnim(EntityID, Anim);
/* AMXX-Studio Notes - DO NOT MODIFY BELOW HERE
*{\\ rtf1\\ ansi\\ deff0{\\ fonttbl{\\ f0\\ fnil Tahoma;}}\n\\ viewkind4\\ uc1\\ pard\\ lang2057\\ f0\\ fs16 \n\\ par }
*/
