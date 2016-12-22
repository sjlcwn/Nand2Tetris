#include "VariableInfo.h"
void VIDestructor(VariableInfo tValue)
{
	if (tValue.pcName)
		delete[] tValue.pcName;
}