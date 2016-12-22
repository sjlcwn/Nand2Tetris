#include "ClassData.h"

ClassData::ClassData():
	m_pcdList(nullptr),m_pcClassNameSpace(nullptr)
{
}
ClassData::~ClassData()
{
	if (this->m_pcClassNameSpace)
		delete[] this->m_pcClassNameSpace;
	if (this->m_pcdList)
	{
		for (_ClassData* pcdLoop=this->m_pcdList,*pcdBack;pcdLoop;pcdLoop=pcdBack)
		{
			pcdBack=pcdLoop->pcdPrev;
			if (pcdLoop->pcClassName)
			{
				delete[] pcdLoop->pcClassName;
			}
			delete pcdLoop;
		}
	}
}

void ClassData::AddClass(char* pcClassName)
{
	size_t sLength=strlen(pcClassName);
	_ClassData* pcdNew=new _ClassData{new char[sLength+1],0,KeyWord<VariableInfo>(),KeyWord<VariableInfo>(),this->m_pcdList};
	memcpy_s(pcdNew->pcClassName,(sLength+1)*sizeof(char),pcClassName,(sLength+1)*sizeof(char));

	if (this->m_pcClassNameSpace)
	{
		size_t sOldLength=strlen(this->m_pcClassNameSpace);
		char* pcNew=new char[sLength+sOldLength+2];
		memcpy_s(pcNew,(sOldLength+sLength+2)*sizeof(char),this->m_pcClassNameSpace,sOldLength*sizeof(char));
		pcNew[sOldLength]='.';
		memcpy_s(pcNew+sOldLength+1,(sLength+1)*sizeof(char),pcClassName,(sLength+1)*sizeof(char));
	}
	else
	{
		this->m_pcClassNameSpace=new char[sLength+1];
		memcpy_s(this->m_pcClassNameSpace,(sLength+1)*sizeof(char),pcClassName,(sLength+1)*sizeof(char));
	}
	this->m_pcdList=pcdNew;
}
void ClassData::RemoveClass()
{
	if (this->m_pcdList)
	{
		_ClassData* pcdBack=this->m_pcdList;
		this->m_pcdList=pcdBack->pcdPrev;
		if (pcdBack->pcClassName)
		{
			delete[] pcdBack->pcClassName;
		}
		delete pcdBack;

		delete[] this->m_pcClassNameSpace;
		size_t sLength=0;
		for (_ClassData* pcdLoop=this->m_pcdList;pcdLoop;pcdLoop=pcdLoop->pcdPrev)
		{
			if (pcdLoop->pcClassName)
				sLength+=strlen(pcdLoop->pcClassName)+1;
		}
		if (sLength)
		{
			this->m_pcClassNameSpace=new char[sLength+1];
			char* pcCursor=this->m_pcClassNameSpace;
			size_t sCurrentLength;
			for (_ClassData* pcdLoop=this->m_pcdList;pcdLoop;pcdLoop=pcdLoop->pcdPrev)
			{
				if (pcdLoop->pcClassName)
				{
					sCurrentLength=strlen(pcdLoop->pcClassName);
					memcpy_s(pcCursor,sCurrentLength*sizeof(char),pcdLoop->pcClassName,sCurrentLength*sizeof(char));
					pcCursor[sCurrentLength]='.';
					pcCursor+=sCurrentLength+1;
				}
			}
			pcCursor[-1]=0;
		}
		else
		{
			this->m_pcClassNameSpace=nullptr;
		}
	}
}
char const* ClassData::GetClassName() const
{
	return this->m_pcClassNameSpace;
}