#include <climits>
#include <memory.h>
#include <stdlib.h>
#include "Operator.h"

unsigned __int8 GetStageCount(OperatorFeature ofFeature)
{
	return (ofFeature&ofPrevious?1:0)+(ofFeature&ofParameter?1:0)+(ofFeature&ofBehind?1:0);
}

OperatorFeature GetStage(OperatorFeature ofFeature,unsigned __int8 ui8Stage)
{
	if (!(ui8Stage--))
	{
		return ofPrevious;
	}
	if (ofFeature&ofParameter&&!(ui8Stage--))
	{
		return ofParameter;
	}
	if (ofFeature&ofBehind&&!(ui8Stage--))
	{
		return ofBehind;
	}
	return 0;
}

unsigned __int8 GetOperator(
	FILE* pfStream,
	TypeInfo tiValueType,
	unsigned long aulLastOperator[2],
	TypeInfo tiLastOperatorType,
	unsigned __int8 bTypeOptional,
	OperatorSet* posOperator,
	unsigned long aulOperator[2],
	void** ppvData)
{
	unsigned long ulLoop[2];
	fpos_t fpPos;
	fgetpos(pfStream,&fpPos);
	for (ulLoop[0]=0;ulLoop[0]<posOperator->ulCount&&(ulLoop[0]<aulLastOperator[0]||ulLoop[0]==aulLastOperator[0]&&posOperator->ppogOperatorGroup[aulLastOperator[0]]->oaAssociativity==oaRight);++ulLoop[0])
	{
		for (ulLoop[1]=0;ulLoop[1]<posOperator->ppogOperatorGroup[ulLoop[0]]->ulCount;++ulLoop[1])
		{
			Operator* poCurrent=posOperator->ppogOperatorGroup[ulLoop[0]]->ppoOperator[ulLoop[1]];
			if (bTypeOptional||((poCurrent->ofFeature&ofPrevious)?1:0)==(tiValueType?1:0))
			{
				if (poCurrent->pmfMatcher[0](pfStream,tiValueType,0,posOperator->pvPublicParameter,poCurrent->ppvStaticParameter?poCurrent->ppvStaticParameter[1]:NULL,ppvData))
				{
					memcpy_s(aulOperator,2*sizeof(unsigned long),ulLoop,2*sizeof(unsigned long));
					return 1;
				}
				else
				{
					fsetpos(pfStream,&fpPos);
				}
			}
		}
	}
	return 0;
}

void AddOperation(OpStack** pposLast,unsigned long aulOperator[2])
{
	OpStack* posNew=(OpStack*)malloc(sizeof(OpStack));
	posNew->pvsValueStack=NULL;
	posNew->ui8Stage=1;
	memcpy_s(posNew->aulOperator,2*sizeof(unsigned long),aulOperator,2*sizeof(unsigned long));
	posNew->posPrev=*pposLast;
	*pposLast=posNew;
}

void AddValue(OpStack* posStack,Type tValue)
{
	ValueStack* pvsValue=(ValueStack*)malloc(sizeof(ValueStack));
	pvsValue->tValue=tValue;
	pvsValue->pvsPrev=posStack->pvsValueStack;
	posStack->pvsValueStack=pvsValue;
}

void FreeValueStack(ValueStack* pvsValue,FreeType ftFreeType)
{
	if (ftFreeType)
	{
		for (ValueStack *pvsLoop=pvsValue,*pvsPrev;pvsLoop;pvsLoop=pvsPrev)
		{
			pvsPrev=pvsLoop->pvsPrev;
			ftFreeType(pvsLoop->tValue);
			free(pvsLoop);
		}
	}
}

void FreeOperatorStack(OpStack** pposStack,FreeType ftFreeType)
{
	OpStack* posBack=(*pposStack)->posPrev;
	FreeValueStack((*pposStack)->pvsValueStack,ftFreeType);
	free(*pposStack);
	*pposStack=posBack;
}

Result OperateStack(Type* ptBuffer,TypeInfo* ptiBufferType,OperatorSet* posOperator,OpStack** pposLast)
{
	Operator* poOperator=posOperator->ppogOperatorGroup[(*pposLast)->aulOperator[0]]->ppoOperator[(*pposLast)->aulOperator[1]];
	OperatorFeature ofStage=GetStage(poOperator->ofFeature,(*pposLast)->ui8Stage);

	if (!ofStage||ofStage==ofBehind&&((poOperator->ofFeature&ofBehindOptional)||*ptiBufferType))
	{
		unsigned __int8 bBehind=!ofStage;
		if (ofStage==ofBehind&&*ptiBufferType)
		{
			AddValue(*pposLast,*ptBuffer);
			*ptiBufferType=0;
			++((*pposLast)->ui8Stage);
			ofStage=0;
			bBehind=1;
		}

		{
			Type* ptValue;
			Type tResult;
			unsigned long ulLoop,ulCount;
			ValueStack* pvsLoop;
			TypeInfo tiInfo;
			unsigned __int8 bResult;
			for (ulCount=0,pvsLoop=(*pposLast)->pvsValueStack;pvsLoop;++ulCount,pvsLoop=pvsLoop->pvsPrev);
			ptValue=(Type*)malloc(ulCount*sizeof(Type));
			for (ulLoop=ulCount-1,pvsLoop=(*pposLast)->pvsValueStack;pvsLoop;--ulLoop,pvsLoop=pvsLoop->pvsPrev)
			{
				ptValue[ulLoop]=pvsLoop->tValue;
			}
			if (posOperator->ctClearType)
			{
				posOperator->ctClearType(&tResult);
			}
			bResult=poOperator->ofOperator(ptValue,ulCount,bBehind,&tResult,posOperator->pvPublicParameter,poOperator->ppvStaticParameter?poOperator->ppvStaticParameter[0]:NULL,(*pposLast)->pvSpace);
			free(ptValue);
			if (!bResult)
			{
				return rFailed;
			}
			else if ((tiInfo=posOperator->gtTypeInfo(tResult))&&*ptiBufferType)
			{
				if (posOperator->ftFreeType)
				{
					posOperator->ftFreeType(tResult);
				}
				return rFailed;
			}
			else if (!*ptiBufferType)
			{
				FreeOperatorStack(pposLast,posOperator->ftFreeType);
				*ptiBufferType=tiInfo;
				*ptBuffer=tResult;
				return rSucceed;
			}
			return bResult==1?rSucceed:rNothing;
		}
	}
	return rNothing;
}

unsigned __int8 Operate(FILE* pfStream,OperatorSet* posOperator,Type* ptResult)
{
	Type tBuffer;
	TypeInfo tiBufferType=0;
	OpStack* posLast=NULL;
	unsigned __int8 bDoLoop=1;
	unsigned __int8 bResult=1;

	posOperator->ctClearType(&tBuffer);

	while (bDoLoop)
	{
		unsigned long aulOperator[2];
		unsigned long aulLastOperator[2]={ULONG_MAX,ULONG_MAX};
		void* pvData=NULL;
		{
			TypeInfo tiInfo=0;
			if (posLast&&GetStage(posOperator->ppogOperatorGroup[posLast->aulOperator[0]]->ppoOperator[posLast->aulOperator[1]]->ofFeature,posLast->ui8Stage)==ofBehind
				&&(tiBufferType||(posOperator->ppogOperatorGroup[posLast->aulOperator[0]]->ppoOperator[posLast->aulOperator[1]]->ofFeature&ofBehindOptional)))
			{
				TypeInfo *ptiInfo;
				unsigned long ulLoop,ulCount;
				ValueStack* pvsLoop;
				memcpy_s(aulLastOperator,2*sizeof(unsigned long),posLast->aulOperator,2*sizeof(unsigned long));
				for (ulCount=1,pvsLoop=posLast->pvsValueStack;pvsLoop;++ulCount,pvsLoop=pvsLoop->pvsPrev);
				ptiInfo=(TypeInfo*)malloc(ulCount*sizeof(TypeInfo));
				for (ulLoop=ulCount-1,pvsLoop=posLast->pvsValueStack;pvsLoop;--ulLoop,pvsLoop=pvsLoop->pvsPrev)
				{
					ptiInfo[ulLoop]=posOperator->gtTypeInfo(pvsLoop->tValue);
				}
				if (tiBufferType)
				{
					ptiInfo[ulCount-1]=posOperator->gtTypeInfo(tBuffer);
					tiInfo=posOperator->ppogOperatorGroup[posLast->aulOperator[0]]->ppoOperator[posLast->aulOperator[1]]->tofTypeOperator(ptiInfo,ulCount,posOperator->pvPublicParameter,posLast->pvSpace);
				}
				else
				{
					tiInfo=posOperator->ppogOperatorGroup[posLast->aulOperator[0]]->ppoOperator[posLast->aulOperator[1]]->tofTypeOperator(ptiInfo,ulCount-1,posOperator->pvPublicParameter,posLast->pvSpace);
				}
				free(ptiInfo);
			}
			bDoLoop=posLast?GetOperator(pfStream,tiBufferType,aulLastOperator,aulLastOperator[0]==ULONG_MAX?0:tiInfo,
				tiInfo&&(posOperator->ppogOperatorGroup[posLast->aulOperator[0]]->ppoOperator[posLast->aulOperator[1]]->ofFeature&ofBehindOptional),
				posOperator,aulOperator,&pvData):
				GetOperator(pfStream,tiBufferType,aulLastOperator,aulLastOperator[0]==ULONG_MAX?0:tiInfo,
				0,
				posOperator,aulOperator,&pvData);
		}
		if (bDoLoop)
		{
			if (tiBufferType&&posLast&&(aulOperator[0]>posLast->aulOperator[0]||
				aulOperator[0]==posLast->aulOperator[0]&&posOperator->ppogOperatorGroup[posLast->aulOperator[0]]->oaAssociativity==oaLeft)&&
				GetStage(posOperator->ppogOperatorGroup[posLast->aulOperator[0]]->ppoOperator[posLast->aulOperator[1]]->ofFeature,posLast->ui8Stage)==ofBehind)
			{
				if (GetStage(posOperator->ppogOperatorGroup[posLast->aulOperator[0]]->ppoOperator[posLast->aulOperator[1]]->ofFeature,posLast->ui8Stage)==ofBehind)
				{
					++(posLast->ui8Stage);
				}
				AddValue(posLast,tBuffer);
				tiBufferType=0;
			}
			AddOperation(&posLast,aulOperator);
			posLast->pvSpace=pvData;
			if (tiBufferType)
			{
				AddValue(posLast,tBuffer);
				tiBufferType=0;
			}
			while (posLast&&!GetStage(posOperator->ppogOperatorGroup[posLast->aulOperator[0]]->ppoOperator[posLast->aulOperator[1]]->ofFeature,posLast->ui8Stage)&&(OperateStack(&tBuffer,&tiBufferType,posOperator,&posLast)==rSucceed));
		}
		else
		{
			if (posLast&&(OperateStack(&tBuffer,&tiBufferType,posOperator,&posLast)==rSucceed))
			{
				bResult=1;
				bDoLoop=1;
			}
			else if (tiBufferType)
			{
				OpStack* posLoop;
				OperatorFeature ofStage;
				for (posLoop=posLast;posLoop&&(ofStage=GetStage(posOperator->ppogOperatorGroup[posLoop->aulOperator[0]]->ppoOperator[posLoop->aulOperator[1]]->ofFeature,posLoop->ui8Stage))==ofBehind;posLoop=posLoop->posPrev);
				if (posLoop&&ofStage==ofParameter)
				{
					fpos_t fpPosition;
					unsigned long ulParameterCount=0;
					aulLastOperator[0]=aulLastOperator[1]=ULONG_MAX;
					fgetpos(pfStream,&fpPosition);
					for (ValueStack* pvsLoop=posLoop->pvsValueStack;pvsLoop;pvsLoop=pvsLoop->pvsPrev,++ulParameterCount);
					{
						Operator* poCurrent=posOperator->ppogOperatorGroup[posLoop->aulOperator[0]]->ppoOperator[posLoop->aulOperator[1]];
						if (poCurrent->pmfMatcher[posLast->ui8Stage](pfStream,tiBufferType,ulParameterCount+1,posOperator->pvPublicParameter,poCurrent->ppvStaticParameter?poCurrent->ppvStaticParameter[posLoop->ui8Stage+1]:NULL,&(posLoop->pvSpace)))
						{
							AddValue(posLoop,tBuffer);
							tiBufferType=0;
							bDoLoop=1;
						}
						else
						{
							fsetpos(pfStream,&fpPosition);
							if (poCurrent->pmfMatcher[posLoop->ui8Stage+1](pfStream,tiBufferType,ulParameterCount+1,posOperator->pvPublicParameter,poCurrent->ppvStaticParameter?poCurrent->ppvStaticParameter[posLoop->ui8Stage+2]:NULL,&(posLoop->pvSpace)))
							{
								while (posLast!=posLoop&&(OperateStack(&tBuffer,&tiBufferType,posOperator,&posLast)==rSucceed));
								if (posLast==posLoop)
								{
									AddValue(posLast,tBuffer);
									tiBufferType=0;
									++(posLast->ui8Stage);
									bDoLoop=1;
									while (!GetStage(posOperator->ppogOperatorGroup[posLast->aulOperator[0]]->ppoOperator[posLast->aulOperator[1]]->ofFeature,posLast->ui8Stage)&&
										OperateStack(&tBuffer,&tiBufferType,posOperator,&posLast)==rSucceed);
								}
							}
							else
							{
								fsetpos(pfStream,&fpPosition);
							}
						}
					}
				}
			}
		}
		if (!bDoLoop)
		{
			{
				char pcSpace;
				fpos_t fpPos;
				fgetpos(pfStream,&fpPos);

				while (strchr(" \r\n",pcSpace=fgetc(pfStream))&&!feof(pfStream));
				if (feof(pfStream)&&(strchr(" \r\n",pcSpace)||pcSpace==EOF))
				{
					while (posLast&&(OperateStack(&tBuffer,&tiBufferType,posOperator,&posLast)==rSucceed));
					if (posLast)
					{
						bResult=0;
					}
				}
				else
				{
					fsetpos(pfStream,&fpPos);
					bResult=0;
				}
			}

			if (!bResult&&!tiBufferType&&GetStage(posOperator->ppogOperatorGroup[posLast->aulOperator[0]]->ppoOperator[posLast->aulOperator[1]]->ofFeature,posLast->ui8Stage)==ofParameter)
			{
				Operator* poLast=posOperator->ppogOperatorGroup[posLast->aulOperator[0]]->ppoOperator[posLast->aulOperator[1]];
				unsigned long ulParameterCount=0;
				fpos_t fpPosition;
				fgetpos(pfStream,&fpPosition);
				for (ValueStack* pvsLoop=posLast->pvsValueStack;pvsLoop;pvsLoop=pvsLoop->pvsPrev,++ulParameterCount);

				if (poLast->pmfMatcher[posLast->ui8Stage+1](pfStream,tiBufferType,ulParameterCount,posOperator->pvPublicParameter,poLast->ppvStaticParameter?poLast->ppvStaticParameter[posLast->ui8Stage+2]:NULL,&(posLast->pvSpace)))
				{
					++(posLast->ui8Stage);
					bResult=1;
					bDoLoop=1;
					while (!GetStage(posOperator->ppogOperatorGroup[posLast->aulOperator[0]]->ppoOperator[posLast->aulOperator[1]]->ofFeature,posLast->ui8Stage)&&
						OperateStack(&tBuffer,&tiBufferType,posOperator,&posLast)==rSucceed);
				}
				else
				{
					fsetpos(pfStream,&fpPosition);
				}
			}
		}
	}

	if (!bResult)
	{
		while (posLast)
		{
			FreeOperatorStack(&posLast,posOperator->ftFreeType);
		}
		if (tiBufferType&&posOperator->ftFreeType)
		{
			posOperator->ftFreeType(tBuffer);
		}
		return 0;
	}
	else
	{
		*ptResult=tBuffer;
		return 1;
	}
}