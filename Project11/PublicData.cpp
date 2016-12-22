#include <string.h>

#include "PublicData.h"

PublicData::PublicData():
	m_pfOut(nullptr),m_pkwFunctionParameter(nullptr),m_pkwFunctionVariable(nullptr),m_ui16Static(0),m_ui16Field(0),m_ui16Var(0),m_bFunction(false),m_ulWhile(0),m_ulIf(0)
{
}
PublicData::~PublicData()
{
	if (this->m_pkwFunctionVariable)
		delete this->m_pkwFunctionVariable;
}

void PublicData::FinishFunction()
{
	if (this->m_bFunction)
	{
		fprintf(this->m_pfOut,"%hu\n",this->m_ui16Var);
		this->m_bFunction=false;
	}
	switch (this->m_ui8FunctionType)
	{
	case 1:
		fprintf(this->m_pfOut,"push constant %hu\ncall Memory.alloc 1\npop pointer 0\n",this->m_ui16Field);
		break;
	case 2:
		fprintf(this->m_pfOut,"push argument 0\npop pointer 0\n");
	}
}