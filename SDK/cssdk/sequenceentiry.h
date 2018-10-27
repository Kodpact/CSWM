#ifndef INCLUDE_SEQUENCEENTRY_H
#define INCLUDE_SEQUENCEENTRY_H

typedef struct sequenceEntry sequenceEntry_t;

typedef struct sequenceEntry
{
	char* fileName;
	char* entryName;
	sequenceCommandLine_s*	firstCommand;
	sequenceEntry_t*		nextEntry;
	qboolean				isGlobal;
};

#endif