#ifndef INCLUDE_ARCHTYPES_H
#define INCLUDE_ARCHTYPES_H

#if defined( _WIN32 ) && (! defined( __MINGW32__ ))
	typedef __int8 int8;
	typedef unsigned __int8 uint8;
	typedef __int16 int16;
	typedef unsigned __int16 uint16;
	typedef __int32 int32;
	typedef unsigned __int32 uint32;
	typedef __int64 int64;
	typedef unsigned __int64 uint64;
	typedef __int32 intp;				// intp is an integer that can accomodate a pointer
	typedef unsigned __int32 uintp;		// (ie, sizeof(intp) >= sizeof(int) && sizeof(intp) >= sizeof(void *)
#else
	typedef char int8;
	typedef unsigned char uint8;
	typedef short int16;
	typedef unsigned short uint16;
	typedef int int32;
	typedef unsigned int uint32;
	typedef long long int64;
	typedef unsigned long long uint64;
	typedef int intp;
	typedef unsigned int uintp;
#endif

#endif
