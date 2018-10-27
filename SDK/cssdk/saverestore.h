#ifndef INCLUDE_SAVERESTORE_H
#define INCLUDE_SAVERESTORE_H

typedef struct saverestore_s SAVERESTOREDATA;

#ifdef _WIN32
typedef 
#endif

struct saverestore_s
{
	char		*pBaseData;		// Start of all entity save data
	char		*pCurrentData;	// Current buffer pointer for sequential access
	int			size;			// Current data size
	int			bufferSize;		// Total space for data
	int			tokenSize;		// Size of the linear list of tokens
	int			tokenCount;		// Number of elements in the pTokens table
	char		**pTokens;		// Hash table of entity strings (sparse)
	int			currentIndex;	// Holds a global entity table ID
	int			tableCount;		// Number of elements in the entity table
	int			connectionCount;// Number of elements in the levelList[]
	ENTITYTABLE	*pTable;		// Array of ENTITYTABLE elements (1 for each entity)
	LEVELLIST	levelList[ MAX_LEVEL_CONNECTIONS ];		// List of connections from this level

	// smooth transition
	int			fUseLandmark;
	char		szLandmarkName[20];// landmark we'll spawn near in next level
	vec3_t		vecLandmarkOffset;// for landmark transitions
	float		time;
	char		szCurrentMapName[32];	// To check global entities

}

#ifdef _WIN32
SAVERESTOREDATA
#endif
;

#endif