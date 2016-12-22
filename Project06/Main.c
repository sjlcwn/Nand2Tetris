#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _KeyWord
{
	char* pcKeyWord;
	unsigned __int16 ui16Value;
	struct _KeyWord* pkwNext;
} KeyWord;

typedef struct _Value
{
	union
	{
		char* pcName;
		unsigned __int16 ui16Value;
	} Data;
	unsigned __int8 bName;
} Value;

void OutputOrder(FILE* pfOut,unsigned __int16 ui16Order);

unsigned __int16 Search(KeyWord* pkwFirst,char const* pcKeyWord)
{
	for (;pkwFirst;pkwFirst=pkwFirst->pkwNext)
	{
		if (!strcmp(pcKeyWord,pkwFirst->pcKeyWord))
		{
			return pkwFirst->ui16Value+1;
		}
	}
	return 0;
}

unsigned __int8 AddKeyWord(KeyWord** pkwFirst,KeyWord** pkwLast,char const* pcKeyWord,unsigned __int16 ui16Value)
{
	KeyWord* pkwKeyWord;
	size_t sLength;
	if (pkwFirst&&Search(*pkwFirst,pcKeyWord))
	{
		return 0;
	}

	sLength=strlen(pcKeyWord);
	pkwKeyWord=(KeyWord*)malloc(sizeof(KeyWord));
	pkwKeyWord->pcKeyWord=(char*)malloc(sizeof(char)*(sLength+1));
	pkwKeyWord->ui16Value=ui16Value;
	pkwKeyWord->pkwNext=0;
	memcpy_s(pkwKeyWord->pcKeyWord,sizeof(char)*(sLength+1),pcKeyWord,sizeof(char)*(sLength+1));
	if (*pkwFirst)
	{
		(*pkwLast)->pkwNext=pkwKeyWord;
		(*pkwLast)=pkwKeyWord;
	}
	else
	{
		(*pkwFirst)=pkwKeyWord;
		(*pkwLast)=pkwKeyWord;
	}
	return 1;
}

Value ProcessLine(FILE* pfIn,unsigned __int16* pui16Line,unsigned __int8* pbError,KeyWord** pkwFirst,KeyWord** pkwLast)
{
	char Input[1000];
	char Instruct[1000];
	Value vValue;
	int iStructLength;

	*pbError=0;
	memset(&(vValue.Data),0,sizeof(vValue.Data));
	vValue.bName=1;

	if (fscanf_s(pfIn, " %[^\r\n]", Input, 1000)==1&&sscanf_s(Input, "%[^/]", Instruct, 1000)==1)
	{
		iStructLength=strlen(Instruct);
		while (iStructLength&&Instruct[iStructLength-1]==' ') --iStructLength;
		Instruct[iStructLength]=0;
		if (iStructLength)
		switch (Instruct[0])
		{
		case '/':
			if (Instruct[1] != '/')
			{
				*pbError=1;
			}
			break;
		case '@':
			if ((Instruct[1] >= '0'&&Instruct[1] <= '9')||Instruct[0]=='-' || Instruct[0] == '1')
			{
				sscanf_s(Instruct + 1, "%hd", &vValue.Data.ui16Value);
				vValue.bName = 0;
			}
			else
			{
				unsigned __int8 bR;
				char* pcLoop;
				bR=Instruct[1]=='r'||Instruct[1]=='R';
				if (bR)
				{
					for (pcLoop=(Instruct+2);*pcLoop;++pcLoop)
					{
						if ((*pcLoop)<'0'||(*pcLoop)>'9')
						{
							bR=0;
							break;
						}
				
					}
				}
				if (bR)
				{
					sscanf_s(Instruct + 2, "%hd", &vValue.Data.ui16Value);
					vValue.bName = 0;
				}
				else
				{
					char acName[1000];
					size_t sLength;
					sscanf_s(Instruct + 1, "%s", acName,1000);
					sLength=strlen(acName);
					vValue.Data.pcName = (char*)malloc(sizeof(char)*(sLength + 1));
					memcpy_s(vValue.Data.pcName, sizeof(char)*(sLength + 1), acName, sizeof(char)*(sLength + 1));
					vValue.bName = 1;
				}
			}
			++*pui16Line;
			break;
		case '(':
		{
			char acName[1000];
			if (!sscanf_s(Instruct + 1, "%[^)]", acName,1000)||!AddKeyWord(pkwFirst,pkwLast,acName,(*pui16Line)))
			{
				*pbError=1;
			}
		}
			break;
		default:
		{
			char dst[4], exp[6], jmp[4];
			int iVals;
			vValue.Data.ui16Value=0xE000;
			vValue.bName = 0;
	//dest-----------------------------------------------------------------------------
			jmp[0]=0;
			if (strchr(Instruct, '='))
			{
				iVals = sscanf_s(Instruct, "%[^=]=%[^;];%s",dst,3,exp,5,jmp,4);
			}
			else
			{
				dst[0]=0;
				iVals = sscanf_s(Instruct, "%[^;];%s",exp,6,jmp,4)+1;
			}
			if (strchr(dst,'a')||strchr(dst,'A'))
			{
				vValue.Data.ui16Value|=1<<5;
			}
			if (strchr(dst,'d')||strchr(dst,'D'))
			{
				vValue.Data.ui16Value|=1<<4;
			}
			if (strchr(dst,'m')||strchr(dst,'M'))
			{
				vValue.Data.ui16Value|=1<<3;
			}
	//comp-----------------------------------------------------------------------------
			{
				__int8 bA,bD,bM;
				bA=strchr(exp,'a')||strchr(exp,'A');
				bD=strchr(exp,'d')||strchr(exp,'D');
				bM=strchr(exp,'m')||strchr(exp,'M');
				if (bM)
				{
					vValue.Data.ui16Value|=0x1000;
				}
				if (strchr(exp,'&')||strchr(exp,'|')||strchr(exp,'!'))
				{
					if (strchr(exp,'|'))
					{
						vValue.Data.ui16Value|=0x540;
					}
					else if (strchr(exp,'!'))
					{
						vValue.Data.ui16Value|=bD?0x340:0xC40;
					}
				}
				else
				{
					static unsigned __int8 aaui8OPList[3][3][3]={
						{
							//0,1,-1
							{0x2A,0x3F,0x3A},
							//y,y+1,y-1
							{0x30,0x37,0x32},
							//-y,-y+1,-y-1
							{0x33,0xff,0x23}
						},
						{
							//x,x+1,x-1
							{0x0C,0x1F,0xE},
							//x+y,x+y+1,x+y-1
							{0x02,0x17,0xFF},
							//x-y,x-y+1,x-y-1
							{0x13,0xFF,0x06}
						},
						{
							//-x,-x+1,-x-1
							{0x0F,0xFF,0x0B},
							//-x+y,-x+y+1,-x+y-1
							{0x07,0xFF,0x12},
							//-x-y,-x-y+1,-x-y-1
							{0xFF,0xFF,0x03}
						}
					};
					unsigned __int8 ui8Ins[3];//0,+,-
					char* pcFind;

					pcFind=strchr(exp,'d');
					if (!pcFind)
						pcFind=strchr(exp,'D');
					if (pcFind)
					{
						--pcFind;
						while (pcFind>=exp&&*pcFind==' ')
						{
							--pcFind;
						}
						if (pcFind<exp||*pcFind!='-')
						{
							ui8Ins[0]=1;
						}
						else
						{
							ui8Ins[0]=2;
						}
					}
					else
					{
						ui8Ins[0]=0;
					}

					pcFind=strchr(exp,'a');
					if (!pcFind)
						pcFind=strchr(exp,'A');
					if (!pcFind)
						pcFind=strchr(exp,'m');
					if (!pcFind)
						pcFind=strchr(exp,'M');
					if (pcFind)
					{
						--pcFind;
						while (pcFind>=exp&&*pcFind==' ')
						{
							--pcFind;
						}
						if (pcFind<exp||*pcFind!='-')
						{
							ui8Ins[1]=1;
						}
						else
						{
							ui8Ins[1]=2;
						}
					}
					else
					{
						ui8Ins[1]=0;
					}

					pcFind=strchr(exp,'1');

					if (pcFind)
					{
						--pcFind;
						while (pcFind>=exp&&*pcFind==' ')
						{
							--pcFind;
						}
						if (pcFind<exp||*pcFind!='-')
						{
							ui8Ins[2]=1;
						}
						else
						{
							ui8Ins[2]=2;
						}
					}
					else
					{
						ui8Ins[2]=0;
					}
					vValue.Data.ui16Value|=aaui8OPList[ui8Ins[0]][ui8Ins[1]][ui8Ins[2]]<<6;
					if (aaui8OPList[ui8Ins[0]][ui8Ins[1]][ui8Ins[2]]==0xff)
					{
						*pbError=1;
					}
				}
			}
	//jump-----------------------------------------------------------------------------
			if (iVals==3)
			{
				if (!strcmp(jmp,"JGT"))
				{
					vValue.Data.ui16Value|=0x1;
				}
				else if (!strcmp(jmp,"JEQ"))
				{
					vValue.Data.ui16Value|=0x2;
				}
				else if (!strcmp(jmp,"JGE"))
				{
					vValue.Data.ui16Value|=0x3;
				}
				else if (!strcmp(jmp,"JLT"))
				{
					vValue.Data.ui16Value|=0x4;
				}
				else if (!strcmp(jmp,"JNE"))
				{
					vValue.Data.ui16Value|=0x5;
				}
				else if (!strcmp(jmp,"JLE"))
				{
					vValue.Data.ui16Value|=0x6;
				}
				else if (!strcmp(jmp,"JMP"))
				{
					vValue.Data.ui16Value|=0x7;
				}
			}
			if (!*pbError)
			{
				++*pui16Line;
			}
		}
		};
	}
	return vValue;
}

typedef struct _Orders
{
	Value vValue;
	struct _Orders* poNext;
} Orders;

unsigned __int8 AddOrder(Orders** poFirst,Orders** poLast,Value vValue)
{
	Orders* poNew;
	poNew=(Orders*)malloc(sizeof(Orders));
	poNew->vValue=vValue;
	poNew->poNext=0;
	if (*poFirst)
	{
		(*poLast)->poNext=poNew;
		(*poLast)=poNew;
	}
	else
	{
		(*poFirst)=poNew;
		(*poLast)=poNew;
	}
	return 1;
}

void OutputOrder(FILE* pfOut,unsigned __int16 ui16Order)
{
	fprintf_s(pfOut,"%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
		ui16Order&(1<<15)?'1':'0',
		ui16Order&(1<<14)?'1':'0',
		ui16Order&(1<<13)?'1':'0',
		ui16Order&(1<<12)?'1':'0',
		ui16Order&(1<<11)?'1':'0',
		ui16Order&(1<<10)?'1':'0',
		ui16Order&(1<<9)?'1':'0',
		ui16Order&(1<<8)?'1':'0',
		ui16Order&(1<<7)?'1':'0',
		ui16Order&(1<<6)?'1':'0',
		ui16Order&(1<<5)?'1':'0',
		ui16Order&(1<<4)?'1':'0',
		ui16Order&(1<<3)?'1':'0',
		ui16Order&(1<<2)?'1':'0',
		ui16Order&(1<<1)?'1':'0',
		ui16Order&(1<<0)?'1':'0');
}

unsigned __int8 Compile(char const* pcIn,char const* pcOut)
{
	Value vResult;
	unsigned __int16 ui16Line=0;
	unsigned __int8 bError=0;
	KeyWord *pkwFirst=0,*pkwLast=0;
	Orders *poFirst=0,*poLast=0,*poLoop;
	unsigned __int16 ui16Order,ui16Varaddress;
	FILE *pfIn,*pfOut;
	fopen_s(&pfIn,pcIn,"r");
	fopen_s(&pfOut,pcOut,"w+");
	if (!pfIn||!pfOut)
	{
		if (pfIn)
		{
			fclose(pfIn);
		}
		if (pfOut)
		{
			fclose(pfOut);
		}
		return 0;
	}
	
	AddKeyWord(&pkwFirst,&pkwLast,"SCREEN",0x4000);
	AddKeyWord(&pkwFirst,&pkwLast,"KBD",0x6000);
	AddKeyWord(&pkwFirst,&pkwLast,"SP",0x0000);
	AddKeyWord(&pkwFirst,&pkwLast,"LCL",0x0001);
	AddKeyWord(&pkwFirst,&pkwLast,"ARG",0x0002);
	AddKeyWord(&pkwFirst,&pkwLast,"THIS",0x0003);
	AddKeyWord(&pkwFirst,&pkwLast,"THAT",0x0004);

	//Input
	while (!feof(pfIn))
	{
		vResult=ProcessLine(pfIn,&ui16Line,&bError,&pkwFirst,&pkwLast);
		if (!bError)
		{
			if (!vResult.bName||vResult.Data.pcName)
			{
				AddOrder(&poFirst,&poLast,vResult);
			}
		}
		else
		{
			printf("Error Line:%u\n",ui16Line);
		}
	}
	fclose(pfIn);
	ui16Line=0;
	bError=0;
	ui16Varaddress=0x10;
	for (poLoop=poFirst;poLoop;poLoop=poLoop->poNext)
	{
		if (poLoop->vValue.bName)
		{
			ui16Order=Search(pkwFirst,poLoop->vValue.Data.pcName);
			if (!ui16Order)
			{
				AddKeyWord(&pkwFirst,&pkwLast,poLoop->vValue.Data.pcName,ui16Varaddress++);
				printf("Add Sign \"%s\" in Line %d\n",poLoop->vValue.Data.pcName,ui16Line);
				ui16Order=ui16Varaddress;
			}
			free(poLoop->vValue.Data.pcName);
			poLoop->vValue.Data.ui16Value=ui16Order-1;
			poLoop->vValue.bName=0;
		}
		++ui16Line;
	}

	if (!bError)
	{
		for (poLoop=poFirst;poLoop;poLoop=poLoop->poNext)
		{
			OutputOrder(pfOut,poLoop->vValue.Data.ui16Value);
			fprintf(pfOut,"\n");
		}
	}
	fclose(pfOut);

	for (;poFirst;poFirst=poLast)
	{
		poLast=poFirst->poNext;
		free(poFirst);
	}

	for (;pkwFirst;pkwFirst=pkwLast)
	{
		pkwLast=pkwFirst->pkwNext;
		free(pkwFirst);
	}
	return !bError;
}

int main()
{
	char acIn[1000],acOut[1000];
	printf("Asm File:");
	gets_s(acIn,1000);
	printf("Hack File:");
	gets_s(acOut,1000);
	printf("\"%s\"->\"%s\"\n",acIn,acOut);
	printf("Compile %s\n",Compile(acIn,acOut)?"Succeeded":"Failed");
	system("pause");
	return 0;
}