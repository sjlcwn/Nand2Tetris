#define _CRT_SECURE_NO_WARNINGS
#include <string.h>

extern "C"
{
#include "Operator.h"
}
#include "PublicData.h"

char const* pcBlockList=" \t\r\n";

TypeInfo TypeGetter(Type tValue)
{
	if (tValue)
	{
		VariableInfo* pviInfo=reinterpret_cast<VariableInfo*>(tValue);
		switch (pviInfo->vtType)
		{
		case vtInt:
		case vtBoolean:
		case vtChar:
		case vtCustomized:
			return 1;
		case vtName:
		{
			VariableInfo* pviInfo=reinterpret_cast<VariableInfo*>(tValue);
			return strchr(pviInfo->pcName,'.')?3:2;
		}
		case vtOpList:
			return 4;
		};
	}
	return 0;
}
void TypeFreer(Type tValue)
{
	if (tValue)
	{
		VariableInfo* pviInfo=reinterpret_cast<VariableInfo*>(tValue);
		VIDestructor(*pviInfo);
		delete[] pviInfo;
	}
}
void TypeClearor(Type* ptValue)
{
	*ptValue=nullptr;
}

TypeInfo TONull(TypeInfo* tiParameter,unsigned long ulParameterCount,void* pvPublicParameter,void* ppvData)
{
	return 0;
}
TypeInfo TOValue(TypeInfo* tiParameter,unsigned long ulParameterCount,void* pvPublicParameter,void* ppvData)
{
	return 1;
}
TypeInfo TOFunction(TypeInfo* tiParameter,unsigned long ulParameterCount,void* pvPublicParameter,void* ppvData)
{
	return 3;
}
TypeInfo TOOperation(TypeInfo* tiParameter,unsigned long ulParameterCount,void* pvPublicParameter,void* ppvData)
{
	return 4;
}

char* CopyStr(char const* pcSource)
{
	if (pcSource)
	{
		size_t sLength=strlen(pcSource);
		char* pcResult=new char[sLength+1];
		memcpy_s(pcResult,(sLength+1)*sizeof(char),pcSource,(sLength+1)*sizeof(char));
		return pcResult;
	}
	else
		return nullptr;
}

void MoveBack(FILE* pfStream)
{
	fpos_t afpPos[2];
	fgetpos(pfStream,&(afpPos[0]));
	fseek(pfStream,0,SEEK_CUR);
	fgetpos(pfStream,&(afpPos[1]));
	if (afpPos[0]==afpPos[1])
		fseek(pfStream,-(long)sizeof(char),SEEK_CUR);
}

unsigned __int8 SkipBlock(FILE* pfStream)
{
	char cSpace=0;
	unsigned __int8 bResult=0;
	while (strchr(pcBlockList,cSpace=fgetc(pfStream)))
		bResult=1;
	if (!(feof(pfStream)||cSpace==EOF))
	{
		MoveBack(pfStream);
	}
	return bResult;
}

unsigned __int8 InstructMatcher(FILE* pfStream,TypeInfo tiValueType,unsigned long ulParameterCount,void* pvPublicParameter,void const* pvStaticParameter,void** ppvData)
{
	fpos_t fpPos;
	char c[2];
	fgetpos(pfStream,&fpPos);
	SkipBlock(pfStream);
	c[0]=fgetc(pfStream);
	if (c[0]=='/')
	{
		switch (fgetc(pfStream))
		{
		case '*':
			c[1]=fgetc(pfStream);
			while (!feof(pfStream)&&c[1]!=EOF&&(c[0]!='*'||c[1]!='/'))
			{
				c[0]=c[1];
				c[1]=fgetc(pfStream);
			}
			return 1;
		case '/':
			fscanf(pfStream,"%*[^\r\n]");
			return 1;
		}
	}
	fsetpos(pfStream,&fpPos);
	return 0;
}

unsigned __int8 ClassOperator(Type* ptValue,unsigned long ulParameterCount,unsigned __int8 bBihind,Type* ptResult,void* pvPublicParameter,void const* pvStaticParameter,void* pvData)
{
	PublicData* ppdData=reinterpret_cast<PublicData*>(pvPublicParameter);
	ppdData->m_cdClasses.RemoveClass();
	return 1;
}
unsigned __int8 ClassMatcher(FILE* pfStream,TypeInfo tiValueType,unsigned long ulParameterCount,void* pvPublicParameter,void const* pvStaticParameter,void** ppvData)
{
	PublicData* ppdData=reinterpret_cast<PublicData*>(pvPublicParameter);
	fpos_t fpPosition;
	fgetpos(pfStream,&fpPosition);
	if (ulParameterCount==0)
	{
		char acSpace[1000];
		fscanf_s(pfStream,"%*[ \t\r\n]%s",acSpace,1000);
		if (!strcmp(acSpace,"class"))
		{
			char pcSpace;
			SkipBlock(pfStream);
			if ((pcSpace=fgetc(pfStream))=='_'||(pcSpace>='a'&&pcSpace<='z')||(pcSpace>='A'&&pcSpace<='Z'))
			{
				MoveBack(pfStream);
				fscanf_s(pfStream,"%[_a-zA-Z0-9]",acSpace,1000);
				if (strchr(" \t\r\n;{",fgetc(pfStream)))
				{
					MoveBack(pfStream);
					ppdData->m_cdClasses.AddClass(acSpace);
					return 1;
				}
			}
		}
		fsetpos(pfStream,&fpPosition);
	}
	return 0;
}

unsigned __int8 FunctionMatcher(FILE* pfStream,TypeInfo tiValueType,unsigned long ulParameterCount,void* pvPublicParameter,void const* pvStaticParameter,void** ppvData)
{
	PublicData* ppdData=reinterpret_cast<PublicData*>(pvPublicParameter);
	fpos_t fpPosition;
	fgetpos(pfStream,&fpPosition);
	if (ulParameterCount==0)
	{
		char acSpace[1000];
		unsigned __int16 ui16Serial=0;
		fscanf_s(pfStream," %s",acSpace,1000);
		if (!strcmp(acSpace,"function"))
			ppdData->m_ui8FunctionType=0;
		else if (!strcmp(acSpace,"constructor"))
			ppdData->m_ui8FunctionType=1;
		else if (!strcmp(acSpace,"method"))
		{
			ppdData->m_ui8FunctionType=2;
			ui16Serial=1;
		}
		else
		{
			fsetpos(pfStream,&fpPosition);
			return 0;
		}
		{
			char pcSpace;
			SkipBlock(pfStream);
			if ((pcSpace=fgetc(pfStream))=='_'||(pcSpace>='a'&&pcSpace<='z')||(pcSpace>='A'&&pcSpace<='Z'))
			{
				MoveBack(pfStream);
				fscanf_s(pfStream,"%[_a-zA-Z0-9]",acSpace,1000);
				SkipBlock(pfStream);
				fscanf_s(pfStream,"%[_a-zA-Z0-9]",acSpace,1000);
				if (fgetc(pfStream)=='(')
				{
					{
						char aacSpace[2][1000];
						if (ppdData->m_pkwFunctionParameter)
							delete ppdData->m_pkwFunctionParameter;
						ppdData->m_pkwFunctionParameter=new KeyWord<VariableInfo>;
						ppdData->m_pkwFunctionParameter->m_Destructor=VIDestructor;
						while (fscanf_s(pfStream," %[_a-zA-Z0-9] %[_a-zA-Z0-9] ,",aacSpace[0],1000,aacSpace[1],1000)==2)
						{
							VariableInfo viInfo{VariableType::vtCustomized,nullptr,ui16Serial++};
							if (!strcmp(aacSpace[0],"int"))
								viInfo.vtType=VariableType::vtInt;
							else if (!strcmp(aacSpace[0],"boolean"))
								viInfo.vtType=VariableType::vtBoolean;
							else if (!strcmp(aacSpace[0],"char"))
								viInfo.vtType=VariableType::vtChar;
							else
								viInfo.pcName=CopyStr(aacSpace[0]);
							ppdData->m_pkwFunctionParameter->AddKeyWord(aacSpace[1],viInfo);
						}
						if (fgetc(pfStream)!=')')
						{
							delete ppdData->m_pkwFunctionParameter;
							ppdData->m_pkwFunctionParameter=nullptr;
							fsetpos(pfStream,&fpPosition);
							return 0;
						}
					}
					if (ppdData->m_pkwFunctionVariable)
						delete ppdData->m_pkwFunctionVariable;
					ppdData->m_pkwFunctionVariable=new KeyWord<VariableInfo>;
					ppdData->m_pkwFunctionVariable->m_Destructor=VIDestructor;
					ppdData->m_bFunction=true;
					ppdData->m_ui16Var=0;
					fprintf(ppdData->m_pfOut,"function %s.%s ",ppdData->m_cdClasses.GetClassName(),acSpace);
					return 1;
				}
			}
		}
		fsetpos(pfStream,&fpPosition);
	}
	return 0;
}

unsigned __int8 VarMatcher(FILE* pfStream,TypeInfo tiValueType,unsigned long ulParameterCount,void* pvPublicParameter,void const* pvStaticParameter,void** ppvData)
{
	PublicData* ppdData=reinterpret_cast<PublicData*>(pvPublicParameter);
	fpos_t fpPosition;
	fgetpos(pfStream,&fpPosition);
	if (ulParameterCount==0&&ppdData->m_bFunction)
	{
		char acSpace[1000];
		fscanf_s(pfStream," %s",acSpace,1000);
		if (!strcmp(acSpace,"var"))
		{
			fscanf_s(pfStream," %[_a-zA-Z0-9]",acSpace,1000);

			VariableInfo viInfo{VariableType::vtCustomized,nullptr,0};
			if (!strcmp(acSpace,"int"))
				viInfo.vtType=VariableType::vtInt;
			else if (!strcmp(acSpace,"boolean"))
				viInfo.vtType=VariableType::vtBoolean;
			else if (!strcmp(acSpace,"char"))
				viInfo.vtType=VariableType::vtChar;
			else
				viInfo.pcName=CopyStr(acSpace);
			while (fscanf_s(pfStream," %[_a-zA-Z0-9] , ",acSpace,1000))
			{
				viInfo.ui16Serial=ppdData->m_ui16Var++;
				ppdData->m_pkwFunctionVariable->AddKeyWord(acSpace,viInfo);
				if (viInfo.pcName)
					viInfo.pcName=CopyStr(viInfo.pcName);
			}
			if (viInfo.pcName)
				delete[] viInfo.pcName;
			if (fgetc(pfStream)!=';')
			{
				MoveBack(pfStream);
			}
			return 1;
		}
		if (acSpace[0]!='/')
		{
			fsetpos(pfStream,&fpPosition);
			ppdData->FinishFunction();
		}
	}
	return 0;
}

unsigned __int8 FieldMatcher(FILE* pfStream,TypeInfo tiValueType,unsigned long ulParameterCount,void* pvPublicParameter,void const* pvStaticParameter,void** ppvData)
{
	PublicData* ppdData=reinterpret_cast<PublicData*>(pvPublicParameter);
	fpos_t fpPosition;
	fgetpos(pfStream,&fpPosition);
	if (ulParameterCount==0)
	{
		char acSpace[1000];
		fscanf_s(pfStream," %s",acSpace,1000);
		if (!strcmp(acSpace,"field"))
		{
			fscanf_s(pfStream," %[_a-zA-Z0-9]",acSpace,1000);

			VariableInfo viInfo{VariableType::vtCustomized,nullptr,0};
			if (!strcmp(acSpace,"int"))
				viInfo.vtType=VariableType::vtInt;
			else if (!strcmp(acSpace,"boolean"))
				viInfo.vtType=VariableType::vtBoolean;
			else if (!strcmp(acSpace,"char"))
				viInfo.vtType=VariableType::vtChar;
			else
				viInfo.pcName=CopyStr(acSpace);
			while (fscanf_s(pfStream," %[_a-zA-Z0-9] , ",acSpace,1000))
			{
				viInfo.ui16Serial=ppdData->m_ui16Field++;
				ppdData->m_cdClasses.m_pcdList->kwField.AddKeyWord(acSpace,viInfo);
				if (viInfo.pcName)
					viInfo.pcName=CopyStr(viInfo.pcName);
			}
			if (viInfo.pcName)
				delete[] viInfo.pcName;
			if (fgetc(pfStream)!=';')
			{
				MoveBack(pfStream);
			}
			return 1;
		}
		fsetpos(pfStream,&fpPosition);
	}
	return 0;
}
unsigned __int8 StaticMatcher(FILE* pfStream,TypeInfo tiValueType,unsigned long ulParameterCount,void* pvPublicParameter,void const* pvStaticParameter,void** ppvData)
{
	PublicData* ppdData=reinterpret_cast<PublicData*>(pvPublicParameter);
	fpos_t fpPosition;
	fgetpos(pfStream,&fpPosition);
	if (ulParameterCount==0)
	{
		char acSpace[1000];
		fscanf_s(pfStream," %s",acSpace,1000);
		if (!strcmp(acSpace,"static"))
		{
			fscanf_s(pfStream," %[_a-zA-Z0-9]",acSpace,1000);

			VariableInfo viInfo{VariableType::vtCustomized,nullptr,0};
			if (!strcmp(acSpace,"int"))
				viInfo.vtType=VariableType::vtInt;
			else if (!strcmp(acSpace,"boolean"))
				viInfo.vtType=VariableType::vtBoolean;
			else if (!strcmp(acSpace,"char"))
				viInfo.vtType=VariableType::vtChar;
			else
				viInfo.pcName=CopyStr(acSpace);
			while (fscanf_s(pfStream," %[_a-zA-Z0-9] , ",acSpace,1000))
			{
				viInfo.ui16Serial=ppdData->m_ui16Static++;
				ppdData->m_cdClasses.m_pcdList->kwStatic.AddKeyWord(acSpace,viInfo);
				if (viInfo.pcName)
					viInfo.pcName=CopyStr(viInfo.pcName);
			}
			if (viInfo.pcName)
				delete[] viInfo.pcName;
			if (fgetc(pfStream)!=';')
			{
				MoveBack(pfStream);
			}
			return 1;
		}
		fsetpos(pfStream,&fpPosition);
	}
	return 0;
}

typedef struct _Var
{
	unsigned __int8 ui8VarPlace;//0:Class 1:Parameter 2:Local
	unsigned __int16 ui16Serial;
	static char const* const acVarName[];
} Var;

char const* const Var::acVarName[]={"this","static","argument","local"};

unsigned __int8 LetOperator(Type* ptValue,unsigned long ulParameterCount,unsigned __int8 bBihind,Type* ptResult,void* pvPublicParameter,void const* pvStaticParameter,void* pvData)
{
	if (ulParameterCount==1||ulParameterCount==2)
	{
		PublicData* ppdData=reinterpret_cast<PublicData*>(pvPublicParameter);
		Var* pvVar=reinterpret_cast<Var*>(pvData);
		switch (ulParameterCount)
		{
		case 1:
			fprintf(ppdData->m_pfOut,"pop %s %hu\n",Var::acVarName[pvVar->ui8VarPlace],pvVar->ui16Serial);
			break;
		case 2:
			fprintf(ppdData->m_pfOut,"pop temp 0\npop pointer 1\npush temp 0\npop that 0\n");
			break;
		}
		delete pvVar;
		return 1;
	}
	delete reinterpret_cast<Var*>(pvData);
	return 0;
}
unsigned __int8 LetMatcher1(FILE* pfStream,TypeInfo tiValueType,unsigned long ulParameterCount,void* pvPublicParameter,void const* pvStaticParameter,void** ppvData)
{
	PublicData* ppdData=reinterpret_cast<PublicData*>(pvPublicParameter);
	fpos_t fpPosition;
	fgetpos(pfStream,&fpPosition);
	if (ulParameterCount==0&&!ppdData->m_bFunction)
	{
		char acSpace[1000];
		fscanf_s(pfStream," %s",acSpace,1000);
		if (!strcmp(acSpace,"let"))
		{
			if (fscanf_s(pfStream," %[_a-zA-Z0-9.] ",acSpace,1000)>0)
			{
				KeyWordStruct<VariableInfo>* pkwsKeyWord;
				char* pcFind;
				SkipBlock(pfStream);
				switch (fgetc(pfStream))
				{
				case '[':
					*ppvData=(void*)1;
				case '=':
					break;
				default:
					fsetpos(pfStream,&fpPosition);
					return 0;
				}
				if (pcFind=strrchr(acSpace,'.'))
				{
					if (pcFind-acSpace==4&&!memcmp(acSpace,"this",4*sizeof(char))&&ppdData->m_cdClasses.m_pcdList)
					{
						if (pkwsKeyWord=ppdData->m_cdClasses.m_pcdList->kwField.SearchKeyWord(pcFind+1))
							*ppvData=new Var{0,pkwsKeyWord->tValue.ui16Serial};
						else if (pkwsKeyWord=ppdData->m_cdClasses.m_pcdList->kwStatic.SearchKeyWord(pcFind+1))
							*ppvData=new Var{1,pkwsKeyWord->tValue.ui16Serial};
						else
							return 0;
						return 1;
					}
				}
				else
				{
					if (ppdData->m_pkwFunctionVariable&&(pkwsKeyWord=ppdData->m_pkwFunctionVariable->SearchKeyWord(acSpace)))
						*ppvData=new Var{3,pkwsKeyWord->tValue.ui16Serial};
					else if (ppdData->m_pkwFunctionParameter&&(pkwsKeyWord=ppdData->m_pkwFunctionParameter->SearchKeyWord(acSpace)))
						*ppvData=new Var{2,pkwsKeyWord->tValue.ui16Serial};
					else if (ppdData->m_cdClasses.m_pcdList&&(pkwsKeyWord=ppdData->m_cdClasses.m_pcdList->kwField.SearchKeyWord(acSpace)))
						*ppvData=new Var{0,pkwsKeyWord->tValue.ui16Serial};
					else if (ppdData->m_cdClasses.m_pcdList&&(pkwsKeyWord=ppdData->m_cdClasses.m_pcdList->kwStatic.SearchKeyWord(acSpace)))
						*ppvData=new Var{1,pkwsKeyWord->tValue.ui16Serial};
					else
					{
						fsetpos(pfStream,&fpPosition);
						return 0;
					}
					return 1;
				}
			}
		}
		fsetpos(pfStream,&fpPosition);
	}
	return 0;
}
unsigned __int8 LetMatcher2(FILE* pfStream,TypeInfo tiValueType,unsigned long ulParameterCount,void* pvPublicParameter,void const* pvStaticParameter,void** ppvData)
{
	PublicData* ppdData=reinterpret_cast<PublicData*>(pvPublicParameter);
	fpos_t fpPosition;
	fgetpos(pfStream,&fpPosition);
	if (ulParameterCount==1&&*ppvData)
	{
		SkipBlock(pfStream);
		if (fgetc(pfStream)==']')
		{
			SkipBlock(pfStream);
			if (fgetc(pfStream)=='=')
			{
				Var* pvVar=reinterpret_cast<Var*>(*ppvData);
				fprintf(ppdData->m_pfOut,"push %s %hu\nadd\n",Var::acVarName[pvVar->ui8VarPlace],pvVar->ui16Serial);
				return 1;
			}
		}
	}
	fsetpos(pfStream,&fpPosition);
	return 0;
}

unsigned __int8 ReturnDefaultOperator(Type* ptValue,unsigned long ulParameterCount,unsigned __int8 bBihind,Type* ptResult,void* pvPublicParameter,void const* pvStaticParameter,void* pvData)
{
	*ptResult=pvData;
	return 1;
}
unsigned __int8 IntegerMatcher(FILE* pfStream,TypeInfo tiValueType,unsigned long ulParameterCount,void* pvPublicParameter,void const* pvStaticParameter,void** ppvData)
{
	PublicData* ppdData=reinterpret_cast<PublicData*>(pvPublicParameter);
	fpos_t fpPos;
	unsigned __int16 ui16Value;
	fgetpos(pfStream,&fpPos);
	if (fscanf(pfStream," %hu ",&ui16Value)>0)
	{
		if (ui16Value&0x8000)
		{
			if (ui16Value==0x8000)
			{
				fputs("push constant 16384\npush constant 16384\nadd\n",ppdData->m_pfOut);
			}
			else
			{
				fprintf(ppdData->m_pfOut,"push constant %hu\nneg\n",(unsigned __int16)-(signed __int16)ui16Value);
			}
		}
		else
		{
			fprintf(ppdData->m_pfOut,"push constant %hu\n",ui16Value);
		}
		*ppvData=new VariableInfo{VariableType::vtInt,nullptr,0};
		return 1;
	}
	fsetpos(pfStream,&fpPos);
	return 0;
}
unsigned __int8 VariableMatcher(FILE* pfStream,TypeInfo tiValueType,unsigned long ulParameterCount,void* pvPublicParameter,void const* pvStaticParameter,void** ppvData)
{
	PublicData* ppdData=reinterpret_cast<PublicData*>(pvPublicParameter);
	fpos_t fpPos;
	char acSpace[1000];
	fgetpos(pfStream,&fpPos);
	if (fscanf_s(pfStream," %[_a-zA-Z0-9] ",acSpace,1000)>0&&
		(acSpace[0]=='_'||acSpace[0]>='a'&&acSpace[0]<='z'||acSpace[0]>='A'&&acSpace[0]<='Z'))
	{
		KeyWordStruct<VariableInfo>* pkwsKeyWord=nullptr;
		if (ppdData->m_pkwFunctionVariable&&(pkwsKeyWord=ppdData->m_pkwFunctionVariable->SearchKeyWord(acSpace)))
		{
			fprintf(ppdData->m_pfOut,"push local %hu\n",pkwsKeyWord->tValue.ui16Serial);
		}
		else if (ppdData->m_pkwFunctionParameter&&(pkwsKeyWord=ppdData->m_pkwFunctionParameter->SearchKeyWord(acSpace)))
		{
			fprintf(ppdData->m_pfOut,"push argument %hu\n",pkwsKeyWord->tValue.ui16Serial);
		}
		else if (ppdData->m_cdClasses.m_pcdList&&(pkwsKeyWord=ppdData->m_cdClasses.m_pcdList->kwField.SearchKeyWord(acSpace)))
		{
			fprintf(ppdData->m_pfOut,"push this %hu\n",pkwsKeyWord->tValue.ui16Serial);
		}
		else if (ppdData->m_cdClasses.m_pcdList&&(pkwsKeyWord=ppdData->m_cdClasses.m_pcdList->kwStatic.SearchKeyWord(acSpace)))
		{
			fprintf(ppdData->m_pfOut,"push static %hu\n",pkwsKeyWord->tValue.ui16Serial);
		}
		else
		{
			if (!strcmp(acSpace,"this"))
			{
				fprintf(ppdData->m_pfOut,"push pointer 0\n");
				*ppvData=new VariableInfo{VariableType::vtCustomized,CopyStr(ppdData->m_cdClasses.GetClassName()),0};
			}
			else if (!strcmp(acSpace,"null"))
			{
				fprintf(ppdData->m_pfOut,"push constant 0\n");
				*ppvData=new VariableInfo{VariableType::vtInt,nullptr,0};
			}
			else if (!strcmp(acSpace,"true"))
			{
				fprintf(ppdData->m_pfOut,"push constant 0\nnot\n");
				*ppvData=new VariableInfo{VariableType::vtBoolean,nullptr,0};
			}
			else if (!strcmp(acSpace,"false"))
			{
				fprintf(ppdData->m_pfOut,"push constant 0\n");
				*ppvData=new VariableInfo{VariableType::vtBoolean,nullptr,0};
			}
			else
				*ppvData=new VariableInfo{VariableType::vtName,CopyStr(acSpace),0};
		}
		if (pkwsKeyWord)
		{
			*ppvData=new VariableInfo{pkwsKeyWord->tValue.vtType,CopyStr(pkwsKeyWord->tValue.pcName),pkwsKeyWord->tValue.ui16Serial};
		}
		return 1;
	}
	fsetpos(pfStream,&fpPos);
	return 0;
}
unsigned __int8 StringMatcher(FILE* pfStream,TypeInfo tiValueType,unsigned long ulParameterCount,void* pvPublicParameter,void const* pvStaticParameter,void** ppvData)
{
	PublicData* ppdData=reinterpret_cast<PublicData*>(pvPublicParameter);
	fpos_t fpPos;
	char acSpace[1000];
	fgetpos(pfStream,&fpPos);
	SkipBlock(pfStream);
	if (fgetc(pfStream)=='\"')
	{
		fscanf_s(pfStream,"%[^\"]\"",acSpace,1000);
		size_t sLength=strlen(acSpace);
		fprintf(ppdData->m_pfOut,"push constant %u\ncall String.new 1\n",sLength);
		for (unsigned long ulLoop=0;ulLoop<sLength;++ulLoop)
		{
			fprintf(ppdData->m_pfOut,"push constant %u\ncall String.appendChar 2\n",acSpace[ulLoop]);
		}
		*ppvData=new VariableInfo{VariableType::vtCustomized,nullptr,0};
		return 1;
	}
	fsetpos(pfStream,&fpPos);
	return 0;
}

unsigned __int8 BasicMatcher(FILE* pfStream,TypeInfo tiValueType,unsigned long ulParameterCount,void* pvPublicParameter,void const* pvStaticParameter,void** ppvData)
{
	if (pvStaticParameter)
	{
		char const** ppcString=(char const**)pvStaticParameter;
		size_t sLength=strlen(ppcString[0]);
		char* pcSpace=(char*)malloc(sLength*sizeof(char));
		fpos_t fpPos;
		fgetpos(pfStream,&fpPos);
		SkipBlock(pfStream);
		if (fread(pcSpace,sizeof(char),sLength,pfStream)==sLength)
		{
			unsigned __int8 bMatch=0;
			if (!memcmp(ppcString[0],pcSpace,sLength*sizeof(char)))
			{
				char cSpace;
				if (feof(pfStream)||EOF==(cSpace=fgetc(pfStream)))
				{
					bMatch=1;
				}
				else
				{
					MoveBack(pfStream);
					if (ppcString[1]&&*(ppcString[1]))
					{
						if (strchr(ppcString[1],cSpace))
							bMatch=1;
					}
					else
					{
						bMatch=1;
					}
					if (bMatch)
					{
						if (ppcString[2]&&*(ppcString[2]))
						{
							if (strchr(ppcString[2],cSpace))
								bMatch=0;
						}
					}
				}
			}
			free(pcSpace);
			if (bMatch)
			{
				return bMatch;
			}
		}
		fsetpos(pfStream,&fpPos);
	}
	return 0;
}

unsigned __int8 PlusOperator(Type* ptValue,unsigned long ulParameterCount,unsigned __int8 bBihind,Type* ptResult,void* pvPublicParameter,void const* pvStaticParameter,void* pvData)
{
	if (ulParameterCount==2)
	{
		PublicData* ppdData=reinterpret_cast<PublicData*>(pvPublicParameter);
		fprintf(ppdData->m_pfOut,"add\n");
		*ptResult=new VariableInfo{VariableType::vtInt,nullptr,0};
		return 1;
	}
	return 0;
}
unsigned __int8 MinusOperator(Type* ptValue,unsigned long ulParameterCount,unsigned __int8 bBihind,Type* ptResult,void* pvPublicParameter,void const* pvStaticParameter,void* pvData)
{
	if (ulParameterCount==2)
	{
		PublicData* ppdData=reinterpret_cast<PublicData*>(pvPublicParameter);
		fprintf(ppdData->m_pfOut,"sub\n");
		*ptResult=new VariableInfo{VariableType::vtInt,nullptr,0};
		return 1;
	}
	return 0;
}
unsigned __int8 NegOperator(Type* ptValue,unsigned long ulParameterCount,unsigned __int8 bBihind,Type* ptResult,void* pvPublicParameter,void const* pvStaticParameter,void* pvData)
{
	if (ulParameterCount==1)
	{
		PublicData* ppdData=reinterpret_cast<PublicData*>(pvPublicParameter);
		fprintf(ppdData->m_pfOut,"neg\n");
		*ptResult=new VariableInfo{VariableType::vtInt,nullptr,0};
		return 1;
	}
	return 0;
}
unsigned __int8 TimesOperator(Type* ptValue,unsigned long ulParameterCount,unsigned __int8 bBihind,Type* ptResult,void* pvPublicParameter,void const* pvStaticParameter,void* pvData)
{
	if (ulParameterCount==2)
	{
		PublicData* ppdData=reinterpret_cast<PublicData*>(pvPublicParameter);
		fprintf(ppdData->m_pfOut,"call Math.multiply 2\n");
		*ptResult=new VariableInfo{VariableType::vtInt,nullptr,0};
		return 1;
	}
	return 0;
}
unsigned __int8 DivideOperator(Type* ptValue,unsigned long ulParameterCount,unsigned __int8 bBihind,Type* ptResult,void* pvPublicParameter,void const* pvStaticParameter,void* pvData)
{
	if (ulParameterCount==2)
	{
		PublicData* ppdData=reinterpret_cast<PublicData*>(pvPublicParameter);
		fprintf(ppdData->m_pfOut,"call Math.divide 2\n");
		*ptResult=new VariableInfo{VariableType::vtInt,nullptr,0};
		return 1;
	}
	return 0;
}
unsigned __int8 EqualOperator(Type* ptValue,unsigned long ulParameterCount,unsigned __int8 bBihind,Type* ptResult,void* pvPublicParameter,void const* pvStaticParameter,void* pvData)
{
	if (ulParameterCount==2)
	{
		PublicData* ppdData=reinterpret_cast<PublicData*>(pvPublicParameter);
		fprintf(ppdData->m_pfOut,"eq\n");
		*ptResult=new VariableInfo{VariableType::vtInt,nullptr,0};
		return 1;
	}
	return 0;
}
unsigned __int8 GreaterOperator(Type* ptValue,unsigned long ulParameterCount,unsigned __int8 bBihind,Type* ptResult,void* pvPublicParameter,void const* pvStaticParameter,void* pvData)
{
	if (ulParameterCount==2)
	{
		PublicData* ppdData=reinterpret_cast<PublicData*>(pvPublicParameter);
		fprintf(ppdData->m_pfOut,"gt\n");
		*ptResult=new VariableInfo{VariableType::vtInt,nullptr,0};
		return 1;
	}
	return 0;
}
unsigned __int8 LessOperator(Type* ptValue,unsigned long ulParameterCount,unsigned __int8 bBihind,Type* ptResult,void* pvPublicParameter,void const* pvStaticParameter,void* pvData)
{
	if (ulParameterCount==2)
	{
		PublicData* ppdData=reinterpret_cast<PublicData*>(pvPublicParameter);
		fprintf(ppdData->m_pfOut,"lt\n");
		*ptResult=new VariableInfo{VariableType::vtInt,nullptr,0};
		return 1;
	}
	return 0;
}
unsigned __int8 AndOperator(Type* ptValue,unsigned long ulParameterCount,unsigned __int8 bBihind,Type* ptResult,void* pvPublicParameter,void const* pvStaticParameter,void* pvData)
{
	if (ulParameterCount==2)
	{
		PublicData* ppdData=reinterpret_cast<PublicData*>(pvPublicParameter);
		fprintf(ppdData->m_pfOut,"and\n");
		*ptResult=new VariableInfo{VariableType::vtBoolean,nullptr,0};
		return 1;
	}
	return 0;
}
unsigned __int8 OrOperator(Type* ptValue,unsigned long ulParameterCount,unsigned __int8 bBihind,Type* ptResult,void* pvPublicParameter,void const* pvStaticParameter,void* pvData)
{
	if (ulParameterCount==2)
	{
		PublicData* ppdData=reinterpret_cast<PublicData*>(pvPublicParameter);
		fprintf(ppdData->m_pfOut,"or\n");
		*ptResult=new VariableInfo{VariableType::vtBoolean,nullptr,0};
		return 1;
	}
	return 0;
}
unsigned __int8 NotOperator(Type* ptValue,unsigned long ulParameterCount,unsigned __int8 bBihind,Type* ptResult,void* pvPublicParameter,void const* pvStaticParameter,void* pvData)
{
	if (ulParameterCount==1)
	{
		PublicData* ppdData=reinterpret_cast<PublicData*>(pvPublicParameter);
		fprintf(ppdData->m_pfOut,"not\n");
		*ptResult=new VariableInfo{VariableType::vtBoolean,nullptr,0};
		return 1;
	}
	return 0;
}
unsigned __int8 DotOperator(Type* ptValue,unsigned long ulParameterCount,unsigned __int8 bBihind,Type* ptResult,void* pvPublicParameter,void const* pvStaticParameter,void* pvData)
{
	VariableInfo** ppviInfo=reinterpret_cast<VariableInfo**>(ptValue);
	char* pcSpace=reinterpret_cast<char*>(pvData);
	if (ulParameterCount==1)
	{
		size_t sLength[2]={ppviInfo[0]->pcName?strlen(ppviInfo[0]->pcName):0,strlen(pcSpace)};
		char* pcNew=new char[sLength[0]+sLength[1]+2];
		memcpy_s(pcNew,sLength[0]+sLength[1]+2,ppviInfo[0]->pcName,sLength[0]);
		pcNew[sLength[0]]='.';
		memcpy_s(pcNew+sLength[0]+1,sLength[1]+1,pcSpace,sLength[1]+1);
		*ptResult=new VariableInfo{VariableType::vtName,pcNew,ppviInfo[0]->vtType!=VariableType::vtName||ppviInfo[0]->ui16Serial};
	}
	delete[] pcSpace;
	return ulParameterCount?1:0;
}
unsigned __int8 DotMatcher(FILE* pfStream,TypeInfo tiValueType,unsigned long ulParameterCount,void* pvPublicParameter,void const* pvStaticParameter,void** ppvData)
{
	PublicData* ppdData=reinterpret_cast<PublicData*>(pvPublicParameter);
	fpos_t fpPosition;
	fgetpos(pfStream,&fpPosition);
	if (ulParameterCount==0&&!ppdData->m_bFunction)
	{
		SkipBlock(pfStream);
		if (fgetc(pfStream)=='.')
		{
			char acSpace[1000];
			fscanf_s(pfStream," %[_a-zA-Z0-9]",acSpace,1000);
			size_t sLength=strlen(acSpace);
			char* pcBack=new char[sLength+1];
			memcpy_s(pcBack,(sLength+1)*sizeof(char),acSpace,(sLength+1)*sizeof(char));
			*ppvData=pcBack;
			return 1;
		}
	}
	fsetpos(pfStream,&fpPosition);
	return 0;
}

unsigned __int8 FunctionCallOperator(Type* ptValue,unsigned long ulParameterCount,unsigned __int8 bBihind,Type* ptResult,void* pvPublicParameter,void const* pvStaticParameter,void* pvData)
{
	PublicData* ppdData=reinterpret_cast<PublicData*>(pvPublicParameter);
	VariableInfo** ppviInfo=reinterpret_cast<VariableInfo**>(ptValue);
	unsigned __int8 ui8Extra=strchr(ppviInfo[0]->pcName,'.')?0:1;
	fprintf(ppdData->m_pfOut,"call %s%s%s %u\n",ui8Extra?ppdData->m_cdClasses.GetClassName():"",ui8Extra?".":"",ppviInfo[0]->pcName,ulParameterCount-(ppviInfo[0]->vtType!=VariableType::vtName||ppviInfo[0]->ui16Serial?0:1)+ui8Extra);
	*ptResult=new VariableInfo{vtCustomized,NULL,0};
	return 1;
}
unsigned __int8 FunctionCallMatcher(FILE* pfStream,TypeInfo tiValueType,unsigned long ulParameterCount,void* pvPublicParameter,void const* pvStaticParameter,void** ppvData)
{
	PublicData* ppdData=reinterpret_cast<PublicData*>(pvPublicParameter);
	unsigned __int8 bResult=BasicMatcher(pfStream,tiValueType,ulParameterCount,pvPublicParameter,pvStaticParameter,ppvData);
	if (bResult&&tiValueType==2)
		fprintf(ppdData->m_pfOut,"push pointer 0\n");
	return bResult;
}
unsigned __int8 DoOperator(Type* ptValue,unsigned long ulParameterCount,unsigned __int8 bBihind,Type* ptResult,void* pvPublicParameter,void const* pvStaticParameter,void* pvData)
{
	switch (ulParameterCount)
	{
	case 1:
	{
		PublicData* ppdData=reinterpret_cast<PublicData*>(pvPublicParameter);
		fprintf(ppdData->m_pfOut,"pop temp 0\n");
	}
	case 0:
		return 1;
	default:
		return 0;
	}
}
unsigned __int8 DoMatcher(FILE* pfStream,TypeInfo tiValueType,unsigned long ulParameterCount,void* pvPublicParameter,void const* pvStaticParameter,void** ppvData)
{
	PublicData* ppdData=reinterpret_cast<PublicData*>(pvPublicParameter);
	fpos_t fpPosition;
	fgetpos(pfStream,&fpPosition);
	if (ulParameterCount==0&&!ppdData->m_bFunction)
	{
		char acSpace[1000];
		fscanf_s(pfStream," %s ",acSpace,1000);
		if (!strcmp(acSpace,"do"))
		{
			return 1;
		}
		fsetpos(pfStream,&fpPosition);
	}
	return 0;
}
unsigned __int8 ReturnOperator(Type* ptValue,unsigned long ulParameterCount,unsigned __int8 bBihind,Type* ptResult,void* pvPublicParameter,void const* pvStaticParameter,void* pvData)
{
	PublicData* ppdData=reinterpret_cast<PublicData*>(pvPublicParameter);
	switch (ulParameterCount)
	{
	case 0:
		fprintf(ppdData->m_pfOut,"push constant 0\n");
	case 1:
		fprintf(ppdData->m_pfOut,"return\n");
		return 1;
	default:
		return 0;
	};
}
unsigned __int8 ReturnMatcher(FILE* pfStream,TypeInfo tiValueType,unsigned long ulParameterCount,void* pvPublicParameter,void const* pvStaticParameter,void** ppvData)
{
	PublicData* ppdData=reinterpret_cast<PublicData*>(pvPublicParameter);
	fpos_t fpPosition;
	fgetpos(pfStream,&fpPosition);
	if (ulParameterCount==0&&!ppdData->m_bFunction)
	{
		char acSpace[1000];
		fscanf_s(pfStream," %[_a-zA-Z0-9]",acSpace,1000);
		if (strchr(" \t\r\n;",fgetc(pfStream))&&!strcmp(acSpace,"return"))
		{
			MoveBack(pfStream);
			return 1;
		}
		fsetpos(pfStream,&fpPosition);
	}
	return 0;
}

unsigned __int8 BracketsOperator(Type* ptValue,unsigned long ulParameterCount,unsigned __int8 bBihind,Type* ptResult,void* pvPublicParameter,void const* pvStaticParameter,void* pvData)
{
	if (ulParameterCount==1)
	{
		VariableInfo** ppviInfo=reinterpret_cast<VariableInfo**>(ptValue);
		PublicData* ppdData=reinterpret_cast<PublicData*>(pvPublicParameter);
		*ptResult=new VariableInfo{ppviInfo[0]->vtType,CopyStr(ppviInfo[0]->pcName),ppviInfo[0]->ui16Serial};
		return 1;
	}
	return 0;
}
unsigned __int8 MBraceOperator(Type* ptValue,unsigned long ulParameterCount,unsigned __int8 bBihind,Type* ptResult,void* pvPublicParameter,void const* pvStaticParameter,void* pvData)
{
	if (ulParameterCount==2)
	{
		fprintf(reinterpret_cast<PublicData*>(pvPublicParameter)->m_pfOut,"add\npop pointer 1\npush that 0\n");
		*ptResult=new VariableInfo{vtCustomized,nullptr,0};
		return 1;
	}
	return 0;
}
unsigned __int8 LBraceOperator(Type* ptValue,unsigned long ulParameterCount,unsigned __int8 bBihind,Type* ptResult,void* pvPublicParameter,void const* pvStaticParameter,void* pvData)
{
	*ptResult=new VariableInfo{vtOpList,nullptr,0};
	return 1;
}

unsigned __int8 WhileOperator(Type* ptValue,unsigned long ulParameterCount,unsigned __int8 bBihind,Type* ptResult,void* pvPublicParameter,void const* pvStaticParameter,void* pvData)
{
	VariableInfo** ppviInfo=reinterpret_cast<VariableInfo**>(ptValue);
	unsigned long ulSpace=*reinterpret_cast<unsigned long*>(pvData);
	delete reinterpret_cast<unsigned long*>(pvData);
	if (ulParameterCount==2&&ppviInfo[1]->vtType==vtOpList)
	{
		_fprintf_p(reinterpret_cast<PublicData*>(pvPublicParameter)->m_pfOut,"goto WHILE_EXP%1$u\nlabel WHILE_END%1$u\n",ulSpace);
		return 1;
	}
	return 0;
}
unsigned __int8 WhileMatcher(FILE* pfStream,TypeInfo tiValueType,unsigned long ulParameterCount,void* pvPublicParameter,void const* pvStaticParameter,void** ppvData)
{
	PublicData* ppdData=reinterpret_cast<PublicData*>(pvPublicParameter);
	fpos_t fpPosition;
	fgetpos(pfStream,&fpPosition);
	if (!ppdData->m_bFunction)
	{
		if (ulParameterCount==0)
		{
			char acSpace[1000];
			fscanf_s(pfStream," %s",acSpace,1000);
			if (!strcmp(acSpace,"while"))
			{
				*ppvData=new unsigned long(ppdData->m_ulWhile);
				fprintf(ppdData->m_pfOut,"label WHILE_EXP%u\n",ppdData->m_ulWhile++);
				return 1;
			}
			fsetpos(pfStream,&fpPosition);
		}
		else if (ulParameterCount==1)
		{
			unsigned long* plSerial=reinterpret_cast<unsigned long*>(*ppvData);
			fprintf(ppdData->m_pfOut,"push constant 0\neq\nif-goto WHILE_END%u\n",*plSerial);
			return 1;
		}
	}
	return 0;
}

unsigned __int8 IfOperator(Type* ptValue,unsigned long ulParameterCount,unsigned __int8 bBihind,Type* ptResult,void* pvPublicParameter,void const* pvStaticParameter,void* pvData)
{
	VariableInfo** ppviInfo=reinterpret_cast<VariableInfo**>(ptValue);
	unsigned long ulSpace=*reinterpret_cast<unsigned long*>(pvData);
	delete reinterpret_cast<unsigned long*>(pvData);
	switch (ulParameterCount)
	{
	case 2:
		fprintf(reinterpret_cast<PublicData*>(pvPublicParameter)->m_pfOut,"label IF_FALSE%u\n",ulSpace);
		break;
	case 3:
		fprintf(reinterpret_cast<PublicData*>(pvPublicParameter)->m_pfOut,"label IF_END%u\n",ulSpace);
		break;
	}
	return 1;
}
unsigned __int8 IfMatcher(FILE* pfStream,TypeInfo tiValueType,unsigned long ulParameterCount,void* pvPublicParameter,void const* pvStaticParameter,void** ppvData)
{
	PublicData* ppdData=reinterpret_cast<PublicData*>(pvPublicParameter);
	unsigned long* plSerial=reinterpret_cast<unsigned long*>(*ppvData);;
	fpos_t fpPosition;
	fgetpos(pfStream,&fpPosition);
	if (!ppdData->m_bFunction)
	{
		switch (ulParameterCount)
		{
		case 0:
		{
			char acSpace[1000];
			fscanf_s(pfStream," %s",acSpace,1000);
			if (!strcmp(acSpace,"if"))
			{
				*ppvData=new unsigned long(ppdData->m_ulIf++);
				return 1;
			}
			fsetpos(pfStream,&fpPosition);
		}
			break;
		case 1:
		{
			_fprintf_p(ppdData->m_pfOut,"if-goto IF_TRUE%1$u\ngoto IF_FALSE%1$u\nlabel IF_TRUE%1$u\n",*plSerial);
			return 1;
		}
			break;
		case 2:
		{
			char acSpace[1000];
			fscanf_s(pfStream," %s",acSpace,1000);
			if (!strcmp(acSpace,"else"))
			{
				_fprintf_p(ppdData->m_pfOut,"goto IF_END%1$u\nlabel IF_FALSE%1$u\n",*plSerial);
				return 1;
			}
			fsetpos(pfStream,&fpPosition);
		}
			break;
		};
	}
	return 0;
}
unsigned __int8 IfEndMatcher(FILE* pfStream,TypeInfo tiValueType,unsigned long ulParameterCount,void* pvPublicParameter,void const* pvStaticParameter,void** ppvData)
{
	return ulParameterCount==3||ulParameterCount==2;
}

char const* apcPlus[3]={"+",NULL,NULL};
void const* apvPlus[]={NULL,apcPlus};
char const* apcMinus[3]={"-",NULL,NULL};
void const* apvMinus[]={NULL,apcMinus};

char const* apcTimes[3]={"*",NULL,NULL};
void const* apvTimes[]={NULL,apcTimes};
char const* apcDivide[3]={"/",NULL,NULL};
void const* apvDivide[]={NULL,apcDivide};

char const* apcAnd[3]={"&",NULL,NULL};
void const* apvAnd[]={NULL,apcAnd};
char const* apcOr[3]={"|",NULL,NULL};
void const* apvOr[]={NULL,apcOr};
char const* apcNot1[3]={"!",NULL,NULL};
void const* apvNot1[]={NULL,apcNot1};
char const* apcNot2[3]={"~",NULL,NULL};
void const* apvNot2[]={NULL,apcNot2};
char const* apcEqual[3]={"=",NULL,NULL};
void const* apvEqual[]={NULL,apcEqual};
char const* apcGreater[3]={">",NULL,NULL};
void const* apvGreater[]={NULL,apcGreater};
char const* apcLess[3]={"<",NULL,NULL};
void const* apvLess[]={NULL,apcLess};

char const* apcSemicolon[3]={";",NULL,NULL};
void const* apvSemicolon[]={NULL,apcSemicolon};

char const* apcComma[3]={",",NULL,NULL};
void const* apvComma[]={NULL,apcComma};

char const* apcDot[3]={".",NULL,NULL};
void const* apvDot[]={NULL,apcDot};

char const* apcLBracket[3]={"(",NULL,NULL};
char const* apcRBracket[3]={")",NULL,NULL};
void const* apvBracket[]={NULL,apcLBracket,NULL,apcRBracket};

char const* apcLMBrace[3]={"[",NULL,NULL};
char const* apcRMBrace[3]={"]",NULL,NULL};
void const* apvMBrace[]={NULL,apcLMBrace,NULL,apcRMBrace};

char const* apcLLBrace[3]={"{",NULL,NULL};
char const* apcRLBrace[3]={"}",NULL,NULL};
void const* apvLBrace[]={NULL,apcLLBrace,NULL,apcRLBrace};


MatcherFunction mfInstructMatcher=InstructMatcher;
Operator oInstruct1={(OperatorFeature)0,TONull,ReturnDefaultOperator,&mfInstructMatcher,NULL};
Operator oInstruct2={ofPrevious,TONull,BracketsOperator,&mfInstructMatcher,NULL};

MatcherFunction mfIntegerMatcher=IntegerMatcher;
Operator oInteger={(OperatorFeature)0,TOValue,ReturnDefaultOperator,&mfIntegerMatcher,NULL};
MatcherFunction mfVariableMatcher=VariableMatcher;
Operator oVariable={(OperatorFeature)0,TOValue,ReturnDefaultOperator,&mfVariableMatcher,NULL};
MatcherFunction mfStringMatcher=StringMatcher;
Operator oString={(OperatorFeature)0,TOValue,ReturnDefaultOperator,&mfStringMatcher,NULL};

MatcherFunction mfBasicMatcher=BasicMatcher;

Operator oPlus={(OperatorFeature)(OperatorFeature::ofPrevious|OperatorFeature::ofBehind),TOValue,PlusOperator,&mfBasicMatcher,apvPlus};
Operator oMinus={(OperatorFeature)(OperatorFeature::ofPrevious|OperatorFeature::ofBehind),TOValue,MinusOperator,&mfBasicMatcher,apvMinus};
Operator oNeg={OperatorFeature::ofBehind,TOValue,NegOperator,&mfBasicMatcher,apvMinus};

Operator oTimes={(OperatorFeature)(OperatorFeature::ofPrevious|OperatorFeature::ofBehind),TOValue,TimesOperator,&mfBasicMatcher,apvTimes};
Operator oDivide={(OperatorFeature)(OperatorFeature::ofPrevious|OperatorFeature::ofBehind),TOValue,DivideOperator,&mfBasicMatcher,apvDivide};

Operator oEqual={(OperatorFeature)(OperatorFeature::ofPrevious|OperatorFeature::ofBehind),TOValue,EqualOperator,&mfBasicMatcher,apvEqual};
Operator oGreater={(OperatorFeature)(OperatorFeature::ofPrevious|OperatorFeature::ofBehind),TOValue,GreaterOperator,&mfBasicMatcher,apvGreater};
Operator oLess={(OperatorFeature)(OperatorFeature::ofPrevious|OperatorFeature::ofBehind),TOValue,LessOperator,&mfBasicMatcher,apvLess};

Operator oAnd={(OperatorFeature)(OperatorFeature::ofPrevious|OperatorFeature::ofBehind),TOValue,AndOperator,&mfBasicMatcher,apvAnd};
Operator oOr={(OperatorFeature)(OperatorFeature::ofPrevious|OperatorFeature::ofBehind),TOValue,OrOperator,&mfBasicMatcher,apvOr};
Operator oNot1={OperatorFeature::ofBehind,TOValue,NotOperator,&mfBasicMatcher,apvNot1};
Operator oNot2={OperatorFeature::ofBehind,TOValue,NotOperator,&mfBasicMatcher,apvNot2};


MatcherFunction mfDotMatcher=DotMatcher;
Operator oDot={OperatorFeature::ofPrevious,TOFunction,DotOperator,&mfDotMatcher,NULL};

MatcherFunction amfBasic[]={BasicMatcher,BasicMatcher,BasicMatcher};
Operator oBracket={OperatorFeature::ofParameter,TOValue,BracketsOperator,amfBasic,apvBracket};
Operator oMBrace={(OperatorFeature)(OperatorFeature::ofPrevious|OperatorFeature::ofParameter),TOValue,MBraceOperator,amfBasic,apvMBrace};
Operator oLBrace={OperatorFeature::ofParameter,TOOperation,LBraceOperator,amfBasic,apvLBrace};

MatcherFunction amfLetMatcher[]={LetMatcher1,LetMatcher2,BasicMatcher};
void const* pvLetMatcher[]={NULL,NULL,NULL,apcSemicolon};
Operator oLet={OperatorFeature::ofParameter,TONull,LetOperator,amfLetMatcher,pvLetMatcher};

MatcherFunction amfDoMatcher[]={DoMatcher,BasicMatcher,BasicMatcher};
void const* pvDoMatcher[]={NULL,NULL,NULL,apcSemicolon};
Operator oDo={OperatorFeature::ofParameter,TONull,DoOperator,amfDoMatcher,pvDoMatcher};

MatcherFunction amfReturnMatcher[]={ReturnMatcher,BasicMatcher,BasicMatcher};
void const* pvReturnMatcher[]={NULL,NULL,NULL,apcSemicolon};
Operator oReturn={OperatorFeature::ofParameter,TONull,ReturnOperator,amfReturnMatcher,pvReturnMatcher};

MatcherFunction amfWhileMatcher[]={WhileMatcher,BasicMatcher,WhileMatcher};
void const* pvWhileMatcher[]={NULL,NULL,NULL,NULL};
Operator oWhile={(OperatorFeature)(OperatorFeature::ofParameter|OperatorFeature::ofBehind),TONull,WhileOperator,amfWhileMatcher,pvWhileMatcher};

MatcherFunction amfIfMatcher[]={IfMatcher,IfMatcher,IfEndMatcher};
Operator oIf={OperatorFeature::ofParameter,TONull,IfOperator,amfIfMatcher,NULL};

void const* apvFunctionCall[]={NULL,apcLBracket,apcComma,apcRBracket};
MatcherFunction amfFunctionCall[]={FunctionCallMatcher,BasicMatcher,BasicMatcher};
Operator oFunctionCall={(OperatorFeature)(OperatorFeature::ofPrevious|OperatorFeature::ofParameter),TOValue,FunctionCallOperator,amfFunctionCall,apvFunctionCall};

MatcherFunction mfFunctionMatcher=FunctionMatcher;
Operator oFunction={OperatorFeature::ofBehind,TONull,ReturnDefaultOperator,&mfFunctionMatcher,NULL};

MatcherFunction mfVarMatcher=VarMatcher;
Operator oVar={(OperatorFeature)0,TONull,ReturnDefaultOperator,&mfVarMatcher,NULL};

MatcherFunction mfFieldMatcher=FieldMatcher;
Operator oField={(OperatorFeature)0,TONull,ReturnDefaultOperator,&mfFieldMatcher,NULL};
MatcherFunction mfStaticMatcher=StaticMatcher;
Operator oStatic={(OperatorFeature)0,TONull,ReturnDefaultOperator,&mfStaticMatcher,NULL};

MatcherFunction mfClassMatcher=ClassMatcher;
Operator oClass={OperatorFeature::ofBehind,TOOperation,ClassOperator,&mfClassMatcher,NULL};

Operator* aoGroup1[]={&oInstruct1,&oInstruct2,&oInteger,&oVariable,&oString,&oBracket,&oMBrace};
OperatorGroup ogGroup1={aoGroup1,sizeof(aoGroup1)/sizeof(Operator*),oaLeft};

Operator* aoGroup2[]={&oDot};
OperatorGroup ogGroup2={aoGroup2,sizeof(aoGroup2)/sizeof(Operator*),oaLeft};

Operator* aoGroup3[]={&oNeg,&oNot1,&oNot2};
OperatorGroup ogGroup3={aoGroup3,sizeof(aoGroup3)/sizeof(Operator*),oaLeft};

Operator* aoGroup4[]={&oPlus,&oMinus};
OperatorGroup ogGroup4={aoGroup4,sizeof(aoGroup4)/sizeof(Operator*),oaLeft};

Operator* aoGroup5[]={&oGreater,&oLess};
OperatorGroup ogGroup5={aoGroup5,sizeof(aoGroup5)/sizeof(Operator*),oaLeft};

Operator* aoGroup6[]={&oEqual};
OperatorGroup ogGroup6={aoGroup6,sizeof(aoGroup6)/sizeof(Operator*),oaLeft};

Operator* aoGroup7[]={&oAnd};
OperatorGroup ogGroup7={aoGroup7,sizeof(aoGroup7)/sizeof(Operator*),oaLeft};

Operator* aoGroup8[]={&oOr};
OperatorGroup ogGroup8={aoGroup8,sizeof(aoGroup8)/sizeof(Operator*),oaLeft};

Operator* aoGroup9[]={&oVar,&oField,&oStatic,&oLet,&oDo,&oReturn,&oWhile,&oIf,&oFunction,&oClass};
OperatorGroup ogGroup9={aoGroup9,sizeof(aoGroup9)/sizeof(Operator*),oaLeft};

Operator* aoGroup10[]={&oFunctionCall};
OperatorGroup ogGroup10={aoGroup10,sizeof(aoGroup10)/sizeof(Operator*),oaLeft};

Operator* aoGroup11[]={&oLBrace};
OperatorGroup ogGroup11={aoGroup11,sizeof(aoGroup11)/sizeof(Operator*),oaLeft};

Operator* aoGroup12[]={&oTimes,&oDivide};
OperatorGroup ogGroup12={aoGroup12,sizeof(aoGroup12)/sizeof(Operator*),oaLeft};

OperatorGroup* apogGroup[]={&ogGroup11,&ogGroup9,&ogGroup1,&ogGroup2,&ogGroup10,&ogGroup12,&ogGroup3,&ogGroup4,&ogGroup5,&ogGroup6,&ogGroup7,&ogGroup8};
OperatorSet osOperatorSet={apogGroup,sizeof(apogGroup)/sizeof(OperatorGroup*),TypeGetter,TypeFreer,TypeClearor};

bool Compile(char const* pcSource,char* pcDest)
{
	PublicData pdData;
	FILE* pfIn,*pfOut;
	Type tResult=nullptr;
	bool bResult;
	fopen_s(&pfIn,pcSource,"rb");
	fopen_s(&pfOut,pcDest,"w");
	pdData.m_pfOut=pfOut;
	osOperatorSet.pvPublicParameter=&pdData;
	bResult=Operate(pfIn,&osOperatorSet,&tResult)!=0;
	TypeFreer(tResult);
	if (!bResult)
	{
		char cSpace;
		while (!feof(pfIn)&&(cSpace=fgetc(pfIn))!=EOF)
			putchar(cSpace);
		printf("\n");
	}
	fclose(pfIn);
	fclose(pfOut);
	return bResult;
}

int main(int argc,char* argv[],char* envp[])
{
	if (argc>1)
	{
		size_t sLength;
		char* pcVM,*pcName,*pcDot,*pcSlash;
		sLength=strlen(argv[1]);

		pcDot=strrchr(argv[1],'.');
		if (!pcDot)
			pcDot=argv[1]+sLength;
		pcSlash=strrchr(argv[1],'\\');
		if (!pcSlash)
			pcSlash=strrchr(argv[1],'/');
		if (!pcSlash)
			pcSlash=argv[1]-1;
		pcVM=(char*)malloc((pcDot-argv[1]+4)*sizeof(char));
		memcpy_s(pcVM,(pcDot-argv[1]+4)*sizeof(char),argv[1],(pcDot-argv[1])*sizeof(char));
		memcpy_s(pcVM+(pcDot-argv[1]),4*sizeof(char),".vm",4*sizeof(char));

		pcName=(char*)malloc((pcDot-pcSlash)*sizeof(char));
		memcpy_s(pcName,(pcDot-pcSlash)*sizeof(char),pcSlash+1,(pcDot-pcSlash-1)*sizeof(char));
		pcName[pcDot-pcSlash-1]=0;

		printf("Source:%s\nDestination:%s\nName:%s\n",argv[1],pcVM,pcName);

		if (Compile(argv[1],pcVM))
		{
			puts("Compile Ended Successfully\n");
		}
		else
		{
			puts("Compile Failed\n");
		}

		free(pcVM);
		free(pcName);
		system("pause");
		/*

		printf("%c\n",Compile("Main.jack","Main.vm")?'t':'f');
		printf("%c\n",Compile("PongGame.jack","PongGame.vm")?'t':'f');
		printf("%c\n",Compile("Bat.jack","Bat.vm")?'t':'f');
		printf("%c\n",Compile("Ball.jack","Ball.vm")?'t':'f');
		*/
	}
}