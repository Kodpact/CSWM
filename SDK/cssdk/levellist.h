#ifndef INCLUDE_LEVELLIST_H
#define INCLUDE_LEVELLIST_H

#define MAX_LEVEL_CONNECTIONS 16		// These are encoded in the lower 16bits of ENTITYTABLE->flags

typedef struct
{
	char		mapName[ 32 ];
	char		landmarkName[ 32 ];
	edict_t		*pentLandmark;
	vec3_t		vecLandmarkOrigin;
} LEVELLIST;

#endif