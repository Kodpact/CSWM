#ifndef INCLUDE_COLORDEF_H
#define INCLUDE_COLORDEF_H

typedef struct
{
	byte r, g, b;
} color24;

typedef struct
{
	unsigned r, g, b, a;
} colorVec;

typedef struct
{
	unsigned short r, g, b, a;
} PackedColorVec;

#endif