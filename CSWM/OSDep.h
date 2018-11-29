#ifndef INCLUDE_OSDEP_H
#define INCLUDE_OSDEP_B

#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/mman.h>
#include <unistd.h>
#include <limits.h>

#define stricmp strcasecmp

typedef unsigned long       DWORD;
typedef int                 BOOL;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef unsigned char		byte;
#endif

#endif