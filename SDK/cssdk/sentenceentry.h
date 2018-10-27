#ifndef INCLUDE_SENTENCEENTRY_H
#define INCLUDE_SENTENCEENTRY_H

typedef struct sentenceEntry sentenceEntry_s;

struct sentenceEntry
{
	char* data;
	sentenceEntry_s*		nextEntry;
	qboolean				isGlobal;
	unsigned int			index;
};

#endif