#ifndef INCLUDE_CRC_H
#define INCLUDE_CRC_H

typedef struct
{
	unsigned int buf[4];
	unsigned int bits[2];
	unsigned char in[64];
} MD5Context_t;

typedef unsigned int CRC32_t;

#endif