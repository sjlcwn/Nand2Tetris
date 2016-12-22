#pragma once
#include <stdlib.h>
#include <string.h>

template <typename tType> struct KeyWordStruct
{
	char* pcKeyWord;
	tType tValue;
	KeyWordStruct<tType>* pkwsNext;
};

template <typename tType> class KeyWord
{
public:
	KeyWord():
		m_pkwFirst(nullptr),m_pkwLast(nullptr)
	{
	}
	KeyWordStruct<tType>* SearchKeyWord(char const* pcKeyWord) const
	{
		for (KeyWordStruct<tType>* pkwsLoop=this->m_pkwFirst;pkwsLoop;pkwsLoop=pkwsLoop->pkwsNext)
		{
			if (!strcmp(pcKeyWord,pkwsLoop->pcKeyWord))
			{
				return pkwsLoop;
			}
		}
		return 0;
	}
	unsigned __int8 AddKeyWord(char const* pcKeyWord,tType tValue)
	{
		KeyWordStruct<tType>* pkwsKeyWord;
		size_t sLength;
		if (this->SearchKeyWord(pcKeyWord))
		{
			return 0;
		}

		sLength=strlen(pcKeyWord);
		pkwsKeyWord=(KeyWordStruct<tType>*)malloc(sizeof(KeyWordStruct<tType>));
		pkwsKeyWord->pcKeyWord=(char*)malloc(sizeof(char)*(sLength+1));
		pkwsKeyWord->tValue=tValue;
		pkwsKeyWord->pkwsNext=0;
		memcpy_s(pkwsKeyWord->pcKeyWord,sizeof(char)*(sLength+1),pcKeyWord,sizeof(char)*(sLength+1));
		if (this->m_pkwFirst)
		{
			this->m_pkwLast->pkwsNext=pkwsKeyWord;
			this->m_pkwLast=pkwsKeyWord;
		}
		else
		{
			this->m_pkwFirst=pkwsKeyWord;
			this->m_pkwLast=pkwsKeyWord;
		}
		return 1;
	}
	void (*m_Destructor)(tType tValue);
private:
	KeyWordStruct<tType> *m_pkwFirst,*m_pkwLast;
};