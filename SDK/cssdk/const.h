#ifndef INCLUDE_CONST_H
#define INCLUDE_CONST_H

// Max # of clients allowed in a server.
#define MAX_CLIENTS				32

// How many bits to use to encode an edict.
#define MAX_EDICT_BITS				11			// # of bits needed to represent max edicts
// Max # of edicts in a level (2048)
#define MAX_EDICTS					(1<<MAX_EDICT_BITS)

// How many data slots to use when in multiplayer (must be power of 2)
#define MULTIPLAYER_BACKUP			64
// Same for single player
#define SINGLEPLAYER_BACKUP			8
//
// Constants shared by the engine and dlls
// This header file included by engine files and DLL files.
// Most came from server.h

// walkmove modes
#define WALKMOVE_NORMAL			0 // normal walkmove
#define WALKMOVE_WORLDONLY		1 // doesn't hit ANY entities, no matter what the solid type
#define WALKMOVE_CHECKONLY		2 // move, but don't touch triggers

// edict->movetype values
#define MOVETYPE_NONE			0		// never moves
//#define MOVETYPE_ANGLENOCLIP	1
//#define MOVETYPE_ANGLECLIP		2
#define MOVETYPE_WALK			3		// Player only - moving on the ground
#define MOVETYPE_STEP			4		// gravity, special edge handling -- monsters use this
#define MOVETYPE_FLY			5		// No gravity, but still collides with stuff
#define MOVETYPE_TOSS			6		// gravity/collisions
#define MOVETYPE_PUSH			7		// no clip to world, push and crush
#define MOVETYPE_NOCLIP			8		// No gravity, no collisions, still do velocity/avelocity
#define MOVETYPE_FLYMISSILE		9		// extra size to monsters
#define MOVETYPE_BOUNCE			10		// Just like Toss, but reflect velocity when contacting surfaces
#define MOVETYPE_BOUNCEMISSILE	11		// bounce w/o gravity
#define MOVETYPE_FOLLOW			12		// track movement of aiment
#define MOVETYPE_PUSHSTEP		13		// BSP model that needs physics/world collisions (uses nearest hull for world collision)

// edict->solid values
// NOTE: Some movetypes will cause collisions independent of SOLID_NOT/SOLID_TRIGGER when the entity moves
// SOLID only effects OTHER entities colliding with this one when they move - UGH!
#define SOLID_NOT				0		// no interaction with other objects
#define SOLID_TRIGGER			1		// touch on edge, but not blocking
#define SOLID_BBOX				2		// touch on edge, block
#define SOLID_SLIDEBOX			3		// touch on edge, but not an onground
#define SOLID_BSP				4		// bsp clip, touch on edge, block
#define SOLID_CUSTOM			5

// edict->deadflag values
#define DEAD_NO					0 // alive
#define DEAD_DYING				1 // playing death animation or still falling off of a ledge waiting to hit ground
#define DEAD_DEAD				2 // dead. lying still.
#define DEAD_RESPAWNABLE		3
#define DEAD_DISCARDBODY		4

#define DAMAGE_NO				0
#define DAMAGE_YES				1
#define DAMAGE_AIM				2

// entity effects
#define EF_BRIGHTFIELD			1	// swirling cloud of particles
#define EF_MUZZLEFLASH 			2	// single frame ELIGHT on entity attachment 0
#define EF_BRIGHTLIGHT 			4	// DLIGHT centered at entity origin
#define EF_DIMLIGHT 			8	// player flashlight
#define EF_INVLIGHT				16	// get lighting from ceiling
#define EF_NOINTERP				32	// don't interpolate the next frame
#define EF_LIGHT				64	// rocket flare glow sprite
#define EF_NODRAW				128	// don't draw entity
#define EF_NIGHTVISION			256 // player nightvision
#define EF_SNIPERLASER			512 // sniper laser effect
#define EF_FIBERCAMERA			1024// fiber camera

// contents of a spot in the world
#define CONTENTS_EMPTY		-1
#define CONTENTS_SOLID		-2
#define CONTENTS_WATER		-3
#define CONTENTS_SLIME		-4
#define CONTENTS_LAVA		-5
#define CONTENTS_SKY		-6
/* These additional contents constants are defined in bspfile.h
#define CONTENTS_ORIGIN		-7		// removed at csg time
#define CONTENTS_CLIP		-8		// changed to contents_solid
#define CONTENTS_CURRENT_0		-9
#define CONTENTS_CURRENT_90		-10
#define CONTENTS_CURRENT_180	-11
#define CONTENTS_CURRENT_270	-12
#define CONTENTS_CURRENT_UP		-13
#define CONTENTS_CURRENT_DOWN	-14

#define CONTENTS_TRANSLUCENT	-15
*/
#define CONTENTS_LADDER		-16

#define CONTENT_FLYFIELD			-17
#define CONTENT_GRAVITY_FLYFIELD	-18
#define CONTENT_FOG					-19

#define CONTENT_EMPTY	-1
#define CONTENT_SOLID	-2
#define CONTENT_WATER	-3
#define CONTENT_SLIME	-4
#define CONTENT_LAVA	-5
#define CONTENT_SKY		-6

// channels
#define CHAN_AUTO			0
#define CHAN_WEAPON			1
#define CHAN_VOICE			2
#define CHAN_ITEM			3
#define CHAN_BODY			4
#define CHAN_STREAM			5			// allocate stream channel from the static or dynamic area
#define CHAN_STATIC			6			// allocate channel from the static area 
#define CHAN_NETWORKVOICE_BASE	7		// voice data coming across the network
#define CHAN_NETWORKVOICE_END	500		// network voice data reserves slots (CHAN_NETWORKVOICE_BASE through CHAN_NETWORKVOICE_END).
#define CHAN_BOT			501			// channel used for bot chatter.

// attenuation values
#define ATTN_NONE		0
#define ATTN_NORM		0.8f
#define ATTN_IDLE		2.f
#define ATTN_STATIC		1.25f

// pitch values
#define PITCH_NORM		100			// non-pitch shifted
#define PITCH_LOW		95			// other values are possible - 0-255, where 255 is very high
#define PITCH_HIGH		120

// volume values
#define VOL_NORM		1.0

// plats
#define PLAT_LOW_TRIGGER	1

// Trains
#define SF_TRAIN_WAIT_RETRIGGER	1
#define SF_TRAIN_START_ON		4		// Train is initially moving
#define SF_TRAIN_PASSABLE		8		// Train is not solid -- used to make water trains

// Break Model Defines

#define BREAK_TYPEMASK	0x4F
#define BREAK_GLASS		0x01
#define BREAK_METAL		0x02
#define BREAK_FLESH		0x04
#define BREAK_WOOD		0x08

#define BREAK_SMOKE		0x10
#define BREAK_TRANS		0x20
#define BREAK_CONCRETE	0x40
#define BREAK_2			0x80

// Colliding temp entity sounds

#define BOUNCE_GLASS	BREAK_GLASS
#define BOUNCE_METAL	BREAK_METAL
#define BOUNCE_FLESH	BREAK_FLESH
#define BOUNCE_WOOD		BREAK_WOOD
#define BOUNCE_SHRAP	0x10
#define BOUNCE_SHELL	0x20
#define BOUNCE_CONCRETE BREAK_CONCRETE
#define BOUNCE_SHOTSHELL 0x80

// Temp entity bounce sound types
#define TE_BOUNCE_NULL		0
#define TE_BOUNCE_SHELL		1
#define TE_BOUNCE_SHOTSHELL	2

#define eoNullEntity		0	// Testing the three types of "entity" for nullity
#define iStringNull		0	// Testing strings for nullity

#define cchMapNameMost		32

#define CBSENTENCENAME_MAX	16
#define CVOXFILESENTENCEMAX	1536	// max number of sentences in game. NOTE: this must match CVOXFILESENTENCEMAX in engine\sound.h

#define GROUP_OP_AND		0
#define GROUP_OP_NAND		1

// Use this instead of ALLOC_STRING on constant strings
#define STRING(offset)		((const char *)(gpGlobals->pStringBase + (unsigned int)(offset)))
#define MAKE_STRING(str)	((uint64)(str) - (uint64)(STRING(0)))

// Dot products for view cone checking
#define VIEW_FIELD_FULL		-1.0		// +-180 degrees
#define VIEW_FIELD_WIDE		-0.7		// +-135 degrees 0.1 // +-85 degrees, used for full FOV checks
#define VIEW_FIELD_NARROW	0.7		// +-45 degrees, more narrow check used to set up ranged attacks
#define VIEW_FIELD_ULTRA_NARROW	0.9		// +-25 degrees, more narrow check used to set up ranged attacks

#define SND_SPAWNING		(1<<8)		// duplicated in protocol.h we're spawing, used in some cases for ambients
#define SND_STOP		(1<<5)		// duplicated in protocol.h stop sound
#define SND_CHANGE_VOL		(1<<6)		// duplicated in protocol.h change sound vol
#define SND_CHANGE_PITCH	(1<<7)		// duplicated in protocol.h change sound pitch

// All monsters need this data
#define DONT_BLEED		-1
#define BLOOD_COLOR_RED		(byte)247
#define BLOOD_COLOR_YELLOW	(byte)195
#define BLOOD_COLOR_GREEN	BLOOD_COLOR_YELLOW

#define GERMAN_GIB_COUNT	4
#define HUMAN_GIB_COUNT		6
#define ALIEN_GIB_COUNT		4

#define LANGUAGE_ENGLISH	0
#define LANGUAGE_GERMAN		1
#define LANGUAGE_FRENCH		2
#define LANGUAGE_BRITISH	3

#define SVC_TEMPENTITY		23
#define SVC_INTERMISSION	30
#define SVC_CDTRACK		32
#define SVC_WEAPONANIM		35
#define SVC_ROOMTYPE		37
#define SVC_DIRECTOR		51

#define VEC_HULL_MIN_Z		Vector(0, 0, -36)
#define VEC_DUCK_HULL_MIN_Z	Vector(0, 0, -18)

#define VEC_HULL_MIN		Vector(-16, -16, -36)
#define VEC_HULL_MAX		Vector(16, 16, 36)

#define VEC_VIEW		Vector(0, 0, 17)

#define VEC_DUCK_HULL_MIN	Vector(-16, -16, -18)
#define VEC_DUCK_HULL_MAX	Vector(16, 16, 32)
#define VEC_DUCK_VIEW		Vector(0, 0, 12)

// These are caps bits to indicate what an object's capabilities (currently used for save/restore and level transitions)
#define FCAP_CUSTOMSAVE			0x00000001
#define FCAP_ACROSS_TRANSITION		0x00000002	// should transfer between transitions
#define FCAP_MUST_SPAWN			0x00000004	// Spawn after restore
#define FCAP_DONT_SAVE			0x80000000	// Don't save this
#define FCAP_IMPULSE_USE		0x00000008	// can be used by the player
#define FCAP_CONTINUOUS_USE		0x00000010	// can be used by the player
#define FCAP_ONOFF_USE			0x00000020	// can be used by the player
#define FCAP_DIRECTIONAL_USE		0x00000040	// Player sends +/- 1 when using (currently only tracktrains)
#define FCAP_MASTER			0x00000080	// Can be used to "master" other entities (like multisource)

// UNDONE: This will ignore transition volumes (trigger_transition), but not the PVS!!!
#define FCAP_FORCE_TRANSITION		0x00000080	// ALWAYS goes across transitions

// for Classify
#define CLASS_NONE		0
#define CLASS_MACHINE		1
#define CLASS_PLAYER		2
#define CLASS_HUMAN_PASSIVE	3
#define CLASS_HUMAN_MILITARY	4
#define CLASS_ALIEN_MILITARY	5
#define CLASS_ALIEN_PASSIVE	6
#define CLASS_ALIEN_MONSTER	7
#define CLASS_ALIEN_PREY	8
#define CLASS_ALIEN_PREDATOR	9
#define CLASS_INSECT		10
#define CLASS_PLAYER_ALLY	11
#define CLASS_PLAYER_BIOWEAPON	12		// hornets and snarks.launched by players
#define CLASS_ALIEN_BIOWEAPON	13		// hornets and snarks.launched by the alien menace
#define CLASS_VEHICLE		14
#define CLASS_BARNACLE		99		// special because no one pays attention to it, and it eats a wide cross-section of creatures.

#define SF_NORESPAWN		(1 << 30)	// set this bit on guns and stuff that should never respawn.

#define DMG_GENERIC		0		// generic damage was done
#define DMG_CRUSH		(1<<0)		// crushed by falling or moving object
#define DMG_BULLET		(1<<1)		// shot
#define DMG_SLASH		(1<<2)		// cut, clawed, stabbed
#define DMG_BURN		(1<<3)		// heat burned
#define DMG_FREEZE		(1<<4)		// frozen
#define DMG_FALL		(1<<5)		// fell too far
#define DMG_BLAST		(1<<6)		// explosive blast damage
#define DMG_CLUB		(1<<7)		// crowbar, punch, headbutt
#define DMG_SHOCK		(1<<8)		// electric shock
#define DMG_SONIC		(1<<9)		// sound pulse shockwave
#define DMG_ENERGYBEAM		(1<<10)		// laser or other high energy beam
#define DMG_NEVERGIB		(1<<12)		// with this bit OR'd in, no damage type will be able to gib victims upon death
#define DMG_ALWAYSGIB		(1<<13)		// with this bit OR'd in, any damage type can be made to gib victims upon death
#define DMG_DROWN		(1<<14)		// Drowning

// time-based damage
#define DMG_TIMEBASED		(~(0x3FFF))	// mask for time-based damage

#define DMG_PARALYZE		(1<<15)		// slows affected creature down
#define DMG_NERVEGAS		(1<<16)		// nerve toxins, very bad
#define DMG_POISON		(1<<17)		// blood poisioning
#define DMG_RADIATION		(1<<18)		// radiation exposure
#define DMG_DROWNRECOVER	(1<<19)		// drowning recovery
#define DMG_ACID		(1<<20)		// toxic chemicals or acid burns
#define DMG_SLOWBURN		(1<<21)		// in an oven
#define DMG_SLOWFREEZE		(1<<22)		// in a subzero freezer
#define DMG_MORTAR		(1<<23)		// Hit by air raid (done to distinguish grenade from mortar)
#define DMG_EXPLOSION		(1<<24)

// these are the damage types that are allowed to gib corpses
#define DMG_GIB_CORPSE		(DMG_CRUSH | DMG_FALL | DMG_BLAST | DMG_SONIC | DMG_CLUB)

// these are the damage types that have client hud art
#define DMG_SHOWNHUD		(DMG_POISON | DMG_ACID | DMG_FREEZE | DMG_SLOWFREEZE | DMG_DROWN | DMG_BURN | DMG_SLOWBURN | DMG_NERVEGAS | DMG_RADIATION | DMG_SHOCK)

// when calling KILLED(), a value that governs gib behavior is expected to be
// one of these three values
#define GIB_NORMAL		0		// gib if entity was overkilled
#define GIB_NEVER		1		// never gib, no matter how much death damage is done ( freezing, etc )
#define GIB_ALWAYS		2		// always gib ( Houndeye Shock, Barnacle Bite )

#endif