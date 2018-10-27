#ifndef INCLUDE_KEYVALUEDATA_H
#define INCLUDE_KEYVALUEDATA_H

// Passed to pfnKeyValue
typedef struct
{
	char		*szClassName;	// in: entity classname
	char		*szKeyName;		// in: name of key
	char		*szValue;		// in: value of key
	qboolean	fHandled;		// out: DLL sets to true if key-value pair was understood
} KeyValueData;

#endif