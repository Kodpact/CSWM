#ifndef INCLUDE_FLAG_H
#define INCLUDE_FLAG_H

// edict->flags
#define FL_FLY					(1<<0)	// Changes the SV_Movestep() behavior to not need to be on ground
#define FL_SWIM					(1<<1)	// Changes the SV_Movestep() behavior to not need to be on ground (but stay in water)
#define FL_CONVEYOR				(1<<2)
#define FL_CLIENT				(1<<3)
#define FL_INWATER				(1<<4)
#define FL_MONSTER				(1<<5)
#define FL_GODMODE				(1<<6)
#define FL_NOTARGET				(1<<7)
#define FL_SKIPLOCALHOST		(1<<8)	// Don't send entity to local host, it's predicting this entity itself
#define FL_ONGROUND				(1<<9)	// At rest / on the ground
#define FL_PARTIALGROUND		(1<<10)	// not all corners are valid
#define FL_WATERJUMP			(1<<11)	// player jumping out of water
#define FL_FROZEN				(1<<12) // Player is frozen for 3rd person camera
#define FL_FAKECLIENT			(1<<13)	// JAC: fake client, simulated server side; don't send network messages to them
#define FL_DUCKING				(1<<14)	// Player flag -- Player is fully crouched
#define FL_FLOAT				(1<<15)	// Apply floating force to this entity when in water
#define FL_GRAPHED				(1<<16) // worldgraph has this ent listed as something that blocks a connection

// UNDONE: Do we need these?
#define FL_IMMUNE_WATER			(1<<17)
#define FL_IMMUNE_SLIME			(1<<18)
#define FL_IMMUNE_LAVA			(1<<19)

#define FL_PROXY				(1<<20)	// This is a spectator proxy
#define FL_ALWAYSTHINK			(1<<21)	// Brush model flag -- call think every frame regardless of nextthink - ltime (for constantly changing velocity/path)
#define FL_BASEVELOCITY			(1<<22)	// Base velocity has been applied this frame (used to convert base velocity into momentum)
#define FL_MONSTERCLIP			(1<<23)	// Only collide in with monsters who have FL_MONSTERCLIP set
#define FL_ONTRAIN				(1<<24) // Player is _controlling_ a train, so movement commands should be ignored on client during prediction.
#define FL_WORLDBRUSH			(1<<25)	// Not moveable/removeable brush entity (really part of the world, but represented as an entity for transparency or something)
#define FL_SPECTATOR			(1<<26) // This client is a spectator, don't run touch functions, etc.
#define FL_CUSTOMENTITY			(1<<29)	// This is a custom entity
#define FL_KILLME				(1<<30)	// This entity is marked for death -- This allows the engine to kill ents at the appropriate time
#define FL_DORMANT				(1<<31)	// Entity is dormant, no updates to client

// SV_EmitSound2 flags
#define SND_EMIT2_NOPAS			(1<<0)	// never to do check PAS
#define SND_EMIT2_INVOKER		(1<<1)	// do not send to the client invoker

// Engine edict->spawnflags
#define SF_NOTINDEATHMATCH		0x0800	// Do not spawn when deathmatch and loading entities from a file

// Goes into globalvars_t.trace_flags
#define FTRACE_SIMPLEBOX		(1<<0)	// Traceline with a simple box

// entity flags
#define EFLAG_SLERP				1	// do studio interpolation of this entity

#endif