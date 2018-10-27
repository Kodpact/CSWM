#ifndef INCLUDE_TRACE_H
#define INCLUDE_TRACE_H

typedef struct
{
	qboolean	allsolid;		// if true, plane is not valid
	qboolean	startsolid;		// if true, the initial point was in a solid area
	qboolean	inopen, inwater;
	float		fraction;		// time completed, 1.0 = didn't hit anything
	vec3_t		endpos;			// final position
	plane_t		plane;			// surface normal at impact
	edict_t	*	ent;			// entity the surface is on
	int			hitgroup;		// 0 == generic, non zero is specific body part
} trace_t;

#endif