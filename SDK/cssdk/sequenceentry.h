#ifndef INCLUDE_SEQUENCEENTRY_H
#define INCLUDE_SEQUENCEENTRY_H

typedef struct sequenceEntry sequenceEntry_s;

struct sequenceEntry
{
	char* fileName;
	char* entryName;
	sequenceCommandLine_t*	firstCommand;
	sequenceEntry_s*		nextEntry;
	qboolean				isGlobal;
};

#endif