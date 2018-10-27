#ifndef INCLUDE_RESOURCE_H
#define INCLUDE_RESOURCE_H

#define MAX_QPATH 64

typedef struct resource_s
{
	char szFileName[MAX_QPATH];
	resourcetype_t type;
	int  nIndex;
	int  nDownloadSize;
	unsigned char ucFlags;

	unsigned char rgucMD5_hash[16];
	unsigned char playernum;

	unsigned char rguc_reserved[32];
	struct resource_s *pNext;
	struct resource_s *pPrev;
} resource_t;

#endif