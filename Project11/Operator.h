#pragma once
#include <stdio.h>
#include <tchar.h>
typedef void* Type;
typedef unsigned long TypeInfo;
//0:NoType

typedef enum _Result
{
	rNothing,
	rSucceed,
	rFailed,
} Result;

struct _ValueStack;

typedef TypeInfo (*GetType)(Type tValue);
typedef void (*FreeType)(Type tValue);
typedef void (*ClearType)(Type* ptValue);
typedef void (*CopyType)(Type tDestination,Type tSource);
typedef TypeInfo (*TypeOperatorFunction)(TypeInfo* tiParameter,unsigned long ulParameterCount,void* pvPublicParameter,void* ppvData);
typedef unsigned __int8 (*OperatorFunction)(Type* ptValue,unsigned long ulParameterCount,unsigned __int8 bBihind,Type* ptResult,void* pvPublicParameter,void const* pvStaticParameter,void* pvData);
typedef unsigned __int8 (*MatcherFunction)(FILE* pfStream,TypeInfo tiValueType,unsigned long ulParameterCount,void* pvPublicParameter,void const* pvStaticParameter,void** ppvData);

typedef enum _OperatorFeature
{
	ofPrevious=1,
	ofParameter=2,
	ofBehind=4,
	ofBehindOptional=8
} OperatorFeature;
typedef struct _Operator
{
	OperatorFeature ofFeature;
	TypeOperatorFunction tofTypeOperator;
	OperatorFunction ofOperator;
	MatcherFunction* pmfMatcher;
	void const** ppvStaticParameter;
} Operator;

typedef enum _OperatorAssociativity
{
	oaLeft=0,
	oaRight=1
} OperatorAssociativity;

typedef struct _OperatorGroup
{
	Operator** ppoOperator;
	unsigned long ulCount;
	OperatorAssociativity oaAssociativity;
} OperatorGroup;
typedef struct _OperatorSet
{
	OperatorGroup** ppogOperatorGroup;
	unsigned long ulCount;
	GetType gtTypeInfo;
	FreeType ftFreeType;
	ClearType ctClearType;
	void* pvPublicParameter;
} OperatorSet;

typedef struct _ValueStack
{
	Type tValue;
	struct _ValueStack* pvsPrev;
} ValueStack;
typedef struct _OpStack
{
	ValueStack* pvsValueStack;
	unsigned __int8 ui8Stage;
	unsigned long aulOperator[2];
	void* pvSpace;
	struct _OpStack* posPrev;
} OpStack;

unsigned __int8 Operate(FILE* pfStream,OperatorSet* posOperator,Type* ptResult);