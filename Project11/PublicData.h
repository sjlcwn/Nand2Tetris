#pragma once
#include <stdio.h>

#include "KeyWord.h"
#include "ClassData.h"
#include "VariableInfo.h"

class PublicData
{
public:
	PublicData();
	~PublicData();

	void FinishFunction();

	FILE* m_pfOut;
	ClassData m_cdClasses;

	KeyWord<_VariableInfo>* m_pkwFunctionParameter;
	KeyWord<_VariableInfo>* m_pkwFunctionVariable;
	unsigned __int16 m_ui16Static;
	unsigned __int16 m_ui16Field;
	unsigned __int16 m_ui16Var;
	bool m_bFunction;
	unsigned __int8 m_ui8FunctionType;
	unsigned long m_ulWhile;
	unsigned long m_ulIf;
};