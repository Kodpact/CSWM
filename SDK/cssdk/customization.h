#ifndef INCLUDE_CUSTOMIZATION_H
#define INCLUDE_CUSTOMIZATION_H

typedef struct customization_s
{
	qboolean bInUse;
	resource_t resource;
	qboolean bTranslated;
	int nUserData1;
	int  nUserData2;
	void *pInfo;
	void *pBuffer;
	struct customization_s *pNext;
} customization_t;

#endif