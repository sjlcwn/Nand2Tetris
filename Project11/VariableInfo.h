#pragma once
typedef enum _VariableType
{
	vtInt=1,
	vtBoolean,
	vtChar,
	vtCustomized,
	vtName,
	vtOpList
} VariableType;

typedef struct _VariableInfo
{
	enum _VariableType vtType;
	char* pcName;
	unsigned __int16 ui16Serial;
} VariableInfo;

void VIDestructor(VariableInfo tValue);