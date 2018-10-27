#ifndef INCLUDE_TYPEDESCRIPTION_H
#define INCLUDE_TYPEDESCRIPTION_H

#define FTYPEDESC_GLOBAL			0x0001		// This field is masked for global entity save/restore

typedef struct
{
	FIELDTYPE		fieldType;
	char			*fieldName;
	int				fieldOffset;
	short			fieldSize;
	short			flags;
} TYPEDESCRIPTION;

#endif