#pragma once
#include "KeyWord.h"
#include "VariableInfo.h"

class ClassData
{
public:
	ClassData();
	~ClassData();

	void AddClass(char* pcClassName);
	void RemoveClass();
	char const* GetClassName() const;

	struct _ClassData
	{
		char* pcClassName;
		unsigned __int16 ui16VariableCount;
		KeyWord<VariableInfo> kwField;
		KeyWord<VariableInfo> kwStatic;
		struct _ClassData* pcdPrev;
	}* m_pcdList;
	char* m_pcClassNameSpace;
};