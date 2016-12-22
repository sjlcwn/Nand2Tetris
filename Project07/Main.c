#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <Windows.h>

typedef struct _OperationList
{
	char* pcOperation;
	struct _OperationList* polNext;
} OperationList;
void AddOperationList(OperationList** const ppolFirst,OperationList** const ppolLast,char const* const pcOperation)
{
	OperationList* polNew=(OperationList*)malloc(sizeof(OperationList));
	size_t sLength=strlen(pcOperation);
	polNew->pcOperation=malloc((sLength+1)*sizeof(char));
	memcpy_s(polNew->pcOperation,(sLength+1)*sizeof(char),pcOperation,(sLength+1)*sizeof(char));
	polNew->polNext=NULL;
	if (*ppolLast)
	{
		(*ppolLast)->polNext=polNew;
		*ppolLast=polNew;
	}
	else
	{
		*ppolFirst=*ppolLast=polNew;
	}
}
typedef enum _Error
{
	Succeed=0,
	InvalidForm,
	Redefine,
	UnknownValue
} Error;

Error FirstProcess(FILE* const pfIn,char const* const pcFileName,OperationList** const ppolFirst,OperationList** const ppolLast,char acFunctionName[1000],unsigned long aulSerial[2],unsigned long aulStatic[2],unsigned __int8* pbFunction)
{
	char Input[1000];
	char Instruct[1000];
	char Operation[10];
	char acOut[1000]={0};
	if (fscanf_s(pfIn," %[^\r\n]",Input,1000)==1&&sscanf_s(Input,"%[^/]",Instruct,1000)==1&&sscanf_s(Instruct,"%[^ ]",Operation,10)==1)
	{
		if (*Operation)
		{
			if (!strcmp(Operation,"function"))
			{
				char acFunctionNameSpace[1000];
				unsigned long ulLocalCount;
				if (sscanf_s(Instruct+strlen(Operation)," %s %lu",acFunctionNameSpace,1000,&ulLocalCount)==2)
				{
					sprintf_s(acOut,1000,"(function.%s)\n@SP\nD=M\n@LCL\nM=D\n@%lu\nD=D+A\n@SP\nM=D",acFunctionNameSpace,ulLocalCount);
					AddOperationList(ppolFirst,ppolLast,acOut);
					strcpy_s(acFunctionName,1000,acFunctionNameSpace);
					aulSerial[0]=0lu;
					if (!strcmp(acFunctionNameSpace,"Sys.init"))
						*pbFunction=1;
				}
				else
					return InvalidForm;
			}
			else if (!strcmp(Operation,"return"))
			{
				AddOperationList(ppolFirst,ppolLast,"@Sys.Return\n0;JMP");
			}
			else if (!strcmp(Operation,"call"))
			{
				char acFunctionName[1000];
				unsigned long ulArgumentCount;
				if (sscanf_s(Instruct+strlen(Operation)," %s %lu",acFunctionName,1000,&ulArgumentCount)==2)
				{
					_sprintf_p(acOut,1000,"@%4$lu\nD=A\n@R14\nM=D\n@function.%3$s\nD=A\n@R15\nM=D\n@Call.%1$s.%2$lu\nD=A\n@Sys.Call\n0;JMP\n(Call.%1$s.%2$lu)\n",pcFileName,aulSerial[1]++,acFunctionName,5lu+ulArgumentCount);

					AddOperationList(ppolFirst,ppolLast,acOut);
				}
				else
					return InvalidForm;
			}
			else if (!strcmp(Operation,"push")||!strcmp(Operation,"pop"))
			{
				char acValueName[9];
				unsigned long ulNumber=0;
				size_t sLength=0;
				if (sscanf_s(Instruct+strlen(Operation)," %s %lu",acValueName,9,&ulNumber)>0)
				{
					if (!strcmp(acValueName,"static"))
					{
						sprintf_s(acOut,1000-sLength,"@%lu\nD=M",ulNumber+aulStatic[0]+16ul);
						if (ulNumber>=aulStatic[1])
						{
							aulStatic[1]=ulNumber+1;
						}
					}
					else if (!strcmp(acValueName,"this"))
						sprintf_s(acOut,1000-sLength,"@THIS\nD=M\n@%lu\nA=D+A\nD=M",ulNumber);
					else if (!strcmp(acValueName,"local"))
						sprintf_s(acOut,1000-sLength,"@LCL\nD=M\n@%lu\nA=D+A\nD=M",ulNumber);
					else if (!strcmp(acValueName,"argument"))
						sprintf_s(acOut,1000-sLength,"@ARG\nD=M\n@%lu\nA=D+A\nD=M",ulNumber);
					else if (!strcmp(acValueName,"that"))
						sprintf_s(acOut,1000-sLength,"@THAT\nD=M\n@%lu\nA=D+A\nD=M",ulNumber);
					else if (!strcmp(acValueName,"constant"))
					{
						if (ulNumber>=0x8000)
						{
							sprintf_s(acOut,1000-sLength,"@%lu\nD=A\nD=D+A%s",(ulNumber&0xffff)>>1,ulNumber&1?"\nD=D+1":"");
						}
						else
						{
							sprintf_s(acOut,1000-sLength,"@%lu\nD=A",ulNumber);
						}
					}
					else if (!strcmp(acValueName,"pointer"))
						sprintf_s(acOut,1000-sLength,"@%lu\nD=A\n@THIS\nA=D+A\nD=M",ulNumber);
					else if (!strcmp(acValueName,"temp"))
						sprintf_s(acOut,1000-sLength,"@%lu\nD=M",ulNumber+5ul);
					else
						return UnknownValue;
					sLength=strlen(acOut);

					if (strcmp(Operation,"pop"))
						sprintf_s(acOut+sLength,1000-sLength,"\n@SP\nA=M\nM=D\n@SP\nM=M+1");
					else
						sprintf_s(acOut+sLength,1000-sLength,"\nD=A\n@R13\nM=D\n@SP\nAM=M-1\nD=M\n@R13\nA=M\nM=D");
					AddOperationList(ppolFirst,ppolLast,acOut);
				}
				else
					return InvalidForm;
			}
			else if (!strcmp(Operation,"add"))
			{
				AddOperationList(ppolFirst,ppolLast,"@SP\nAM=M-1\nD=M\nA=A-1\nM=D+M");
			}
			else if (!strcmp(Operation,"sub"))
			{
				AddOperationList(ppolFirst,ppolLast,"@SP\nAM=M-1\nD=M\nA=A-1\nM=M-D");
			}
			else if (!strcmp(Operation,"neg"))
			{
				AddOperationList(ppolFirst,ppolLast,"@SP\nA=M-1\nM=-M");
			}
			else if (!strcmp(Operation,"eq"))
			{
				_sprintf_p(acOut,1000,"@SP\nAM=M-1\nD=M\nA=A-1\nD=M-D\nM=-1\n@Compare.%1$s.%2$lu\nD;JEQ\n@SP\nA=M-1\nM=0\n(Compare.%1$s.%2$lu)",pcFileName,(*aulSerial)++);
				AddOperationList(ppolFirst,ppolLast,acOut);
			}
			else if (!strcmp(Operation,"gt"))
			{
				_sprintf_p(acOut,1000,"@Compare.%1$s.%2$lu\nD=A\n@R14\nM=D\n@Sys.Compare.GT\n0;JMP\n(Compare.%1$s.%2$lu)",pcFileName,(*aulSerial)++);
				AddOperationList(ppolFirst,ppolLast,acOut);
			}
			else if (!strcmp(Operation,"lt"))
			{
				_sprintf_p(acOut,1000,"@Compare.%1$s.%2$lu\nD=A\n@R14\nM=D\n@Sys.Compare.LT\n0;JMP\n(Compare.%1$s.%2$lu)",pcFileName,(*aulSerial)++);
				AddOperationList(ppolFirst,ppolLast,acOut);
			}
			else if (!strcmp(Operation,"and"))
			{
				AddOperationList(ppolFirst,ppolLast,"@SP\nAM=M-1\nD=M\nA=A-1\nM=D&M");
			}
			else if (!strcmp(Operation,"or"))
			{
				AddOperationList(ppolFirst,ppolLast,"@SP\nAM=M-1\nD=M\nA=A-1\nM=D|M");
			}
			else if (!strcmp(Operation,"not"))
			{
				AddOperationList(ppolFirst,ppolLast,"@SP\nA=M-1\nM=!M");
			}
			else if (!strcmp(Operation,"label"))
			{
				char acLabelName[1000];
				if (sscanf_s(Instruct+strlen(Operation)," %s",acLabelName,1000)==1)
				{
					sprintf_s(acOut,1000,"(Label.%s.%s.%s)",*acFunctionName?"Function":"File",*acFunctionName?acFunctionName:pcFileName,acLabelName);
					AddOperationList(ppolFirst,ppolLast,acOut);
				}
				else
					return InvalidForm;
			}
			else if (!strcmp(Operation,"goto"))
			{
				char acLabelName[1000];
				if (sscanf_s(Instruct+strlen(Operation)," %s",acLabelName,1000)==1)
				{
					sprintf_s(acOut,1000,"@Label.%s.%s.%s\n0;JMP",*acFunctionName?"Function":"File",*acFunctionName?acFunctionName:pcFileName,acLabelName);
					AddOperationList(ppolFirst,ppolLast,acOut);
				}
				else
					return InvalidForm;
			}
			else if (!strcmp(Operation,"if-goto"))
			{
				char acLabelName[1000];
				if (sscanf_s(Instruct+strlen(Operation)," %s",acLabelName,1000)==1)
				{
					sprintf_s(acOut,1000,"@SP\nAM=M-1\nD=M\n@Label.%s.%s.%s\nD;JNE",*acFunctionName?"Function":"File",*acFunctionName?acFunctionName:pcFileName,acLabelName);
					AddOperationList(ppolFirst,ppolLast,acOut);
				}
				else
					return InvalidForm;
			}
			else
				return UnknownValue;
		}
	}
	return Succeed;
}

typedef struct _VMFile
{
	char* pcFilePath;
	char* pcFileName;
} VMFile;

void Translate(VMFile const* const pvmfFileIn,unsigned long ulFileCount,char const* const pcFileOut)
{
	static char const* apcError[]={"Succeed","InvalidForm","Redefine","UnknownValue"};
	FILE* pfFile;
	OperationList *polFirst=0,*polLast=0,*polNext;
	unsigned long aulSerial[2];
	unsigned long aulStatic[2]={0,0};
	char acFunctionName[1000]={0};
	unsigned __int8 bFunction=0;
	Error eError;
	fpos_t fpBack;

	for (unsigned long ulLoop=0;ulLoop<ulFileCount;++ulLoop)
	{
		fopen_s(&pfFile,pvmfFileIn[ulLoop].pcFilePath,"r");
		printf("File Open:%s\n",pvmfFileIn[ulLoop].pcFileName);
		if (pfFile)
		{
			aulSerial[0]=aulSerial[1]=0lu;
			while (!feof(pfFile))
			{
				fgetpos(pfFile,&fpBack);
				if (eError=FirstProcess(pfFile,pvmfFileIn[ulLoop].pcFileName,&polFirst,&polLast,acFunctionName,aulSerial,aulStatic,&bFunction))
				{
					char acSpace[1000];
					printf("Error:%s\n",apcError[eError]);
					fsetpos(pfFile,&fpBack);
					fscanf_s(pfFile," %[^\r\n]",acSpace,1000);
					printf("%s\n",acSpace);
					break;
				}
			}
		}
		else
		{
			printf("Unable to open\n");
		}
		fclose(pfFile);
		aulStatic[0]+=aulStatic[1];
		aulStatic[1]=0;
	}
	fopen_s(&pfFile,pcFileOut,"w");
	if (bFunction)
	{
		fprintf_s(pfFile,"@Sys.End1\nD=A\n@256\nM=D\nD=A\n@ARG\nM=D\n@261\nD=A\n@SP\nM=D\n@LCL\nM=D\n@function.Sys.init\n(Sys.End1)\n0;JMP\n");
	}
	else
	{
		fprintf_s(pfFile,"@SP\nD=M\n@Sys.Skip\nD;JNE\n@256\nD=A\n@SP\nM=D\n(Sys.Skip)\n");
	}
	while (polFirst)
	{
		fprintf_s(pfFile,"%s\n",polFirst->pcOperation);
		polNext=polFirst->polNext;
		free(polFirst->pcOperation);
		free(polFirst);
		polFirst=polNext;
	}
	fprintf_s(pfFile,"(Sys.End2)\n@Sys.End2\n0;JMP\n");
	fprintf_s(pfFile,"(Sys.Compare.GT)\n@SP\nAM=M-1\nD=M\n@Sys.Compare.GT.0\nD;JLT\n@SP\nA=M-1\nD=M\n@Sys.Compare.GT.2\nD;JLE\n@Sys.Compare.GT.1\n0;JMP\n(Sys.Compare.GT.0)\n@SP\nA=M-1\nD=M\n@Sys.Compare.GT.3\nD;JGE\n(Sys.Compare.GT.1)\n@SP\nA=M\nD=M\nA=A-1\nD=M-D\n@Sys.Compare.GT.3\nD;JGT\n(Sys.Compare.GT.2)\n@SP\nA=M-1\nM=0\n@Sys.Compare.GT.4\n0;JMP\n(Sys.Compare.GT.3)\n@SP\nA=M-1\nM=-1\n(Sys.Compare.GT.4)\n@R14\nA=M\n0;JMP\n");
	fprintf_s(pfFile,"(Sys.Compare.LT)\n@SP\nAM=M-1\nD=M\n@Sys.Compare.LT.0\nD;JLT\n@SP\nA=M-1\nD=M\n@Sys.Compare.LT.3\nD;JLT\n@Sys.Compare.LT.1\n0;JMP\n(Sys.Compare.LT.0)\n@SP\nA=M-1\nD=M\n@Sys.Compare.LT.2\nD;JGE\n(Sys.Compare.LT.1)\n@SP\nA=M\nD=M\nA=A-1\nD=M-D\n@Sys.Compare.LT.3\nD;JLT\n(Sys.Compare.LT.2)\n@SP\nA=M-1\nM=0\n@Sys.Compare.LT.4\n0;JMP\n(Sys.Compare.LT.3)\n@SP\nA=M-1\nM=-1\n(Sys.Compare.LT.4)\n@R14\nA=M\n0;JMP\n");
	fprintf_s(pfFile,"(Sys.Return)\n@5\nD=A\n@LCL\nA=M-D\nD=M\n@R13\nM=D\n@SP\nA=M-1\nD=M\n@ARG\nA=M\nM=D\nD=A\n@SP\nM=D+1\n@LCL\nAM=M-1\nD=M\n@THAT\nM=D\n@LCL\nAM=M-1\nD=M\n@THIS\nM=D\n@LCL\nAM=M-1\nD=M\n@ARG\nM=D\n@LCL\nA=M-1\nD=M\n@LCL\nM=D\n@R13\nA=M\n0;JMP\n");
	//fprintf_s(pfFile,"(Sys.CallBackup)\n@LCL\nD=M\n@SP\nAM=M+1\nM=D\n@ARG\nD=M\n@SP\nAM=M+1\nM=D\n@THIS\nD=M\n@SP\nAM=M+1\nM=D\n@THAT\nD=M\n@SP\nAM=M+1\nM=D\n@R14\nA=M\n0;JMP\n");
	fprintf_s(pfFile,"(Sys.Call)\n@SP\nA=M\nM=D\n@LCL\nD=M\n@SP\nAM=M+1\nM=D\n@ARG\nD=M\n@SP\nAM=M+1\nM=D\n@THIS\nD=M\n@SP\nAM=M+1\nM=D\n@THAT\nD=M\n@SP\nAM=M+1\nM=D\n@SP\nMD=M+1\n@R14\nD=D-M\n@ARG\nM=D\n@R15\nA=M\n0;JMP\n");
	fclose(pfFile);
	while (polFirst)
	{
		polNext=polFirst->polNext;
		free(polFirst->pcOperation);
		free(polFirst);
		polFirst=polNext;
	}
}
//*
int main(int argc,char* argv[],char* envp[])
{
	if (argc>1)
	{
		size_t sLength;
		char* pcDot,*pcSlash;
		char acDst[1000];
		int iLoop;

		VMFile* pvmfFile=(VMFile*)malloc((argc-1)*sizeof(VMFile));
		for (iLoop=1;iLoop<argc;++iLoop)
		{
			sLength=strlen(argv[iLoop]);
			pcDot=strrchr(argv[iLoop],'.');
			if (!pcDot)
				pcDot=argv[iLoop]+sLength;
			pcSlash=strrchr(argv[iLoop],'\\');
			if (!pcSlash)
				pcSlash=strrchr(argv[iLoop],'/');
			if (!pcSlash)
				pcSlash=argv[iLoop]-1;
			pvmfFile[iLoop-1].pcFilePath=argv[iLoop];
			pvmfFile[iLoop-1].pcFileName=(char*)malloc((pcDot-pcSlash)*sizeof(char));
			memcpy_s(pvmfFile[iLoop-1].pcFileName,(pcDot-pcSlash)*sizeof(char),pcSlash+1,(pcDot-pcSlash-1)*sizeof(char));
			pvmfFile[iLoop-1].pcFileName[pcDot-pcSlash-1]=0;
		}
		printf("Dest:");
		scanf_s("%s",acDst,1000);

		Translate(pvmfFile,(unsigned long)(argc-1),acDst);

		for (iLoop=0;iLoop<argc-1;++iLoop)
		{
			free(pvmfFile[iLoop].pcFileName);
		}
		free(pvmfFile);

		system("pause");
	}
}
//*/
/*
void main()
{
	{
		VMFile vmfFile[]={{"Output1\\SimpleAdd.vm","SimpleAdd"}};
		Translate(vmfFile,sizeof(vmfFile)/sizeof(VMFile),"Output1\\SimpleAdd.asm");
	}
}
*/
/*
void main()
{

	{
		VMFile vmfFile[]={{"Output1\\BasicTest.vm","BasicTest"}};
		Translate(vmfFile,sizeof(vmfFile)/sizeof(VMFile),"Output1\\BasicTest.asm");
	}
	{
		VMFile vmfFile[]={{"Output1\\PointerTest.vm","PointerTest"}};
		Translate(vmfFile,sizeof(vmfFile)/sizeof(VMFile),"Output1\\PointerTest.asm");
	}
	{
		VMFile vmfFile[]={{"Output1\\SimpleAdd.vm","SimpleAdd"}};
		Translate(vmfFile,sizeof(vmfFile)/sizeof(VMFile),"Output1\\SimpleAdd.asm");
	}
	{
		VMFile vmfFile[]={{"Output1\\StackTest.vm","StackTest"}};
		Translate(vmfFile,sizeof(vmfFile)/sizeof(VMFile),"Output1\\StackTest.asm");
	}
	{
		VMFile vmfFile[]={{"Output1\\StaticTest.vm","StaticTest"}};
		Translate(vmfFile,sizeof(vmfFile)/sizeof(VMFile),"Output1\\StaticTest.asm");
	}
	{
		VMFile vmfFile[]={{"Output1\\StaticTest.vm","StaticTest"}};
		Translate(vmfFile,sizeof(vmfFile)/sizeof(VMFile),"Output1\\StaticTest.asm");
	}

	{
		VMFile vmfFile[]={
			{"OutPut2\\FunctionCalls\\FibonacciElement\\Sys.vm","Sys"},
			{"OutPut2\\FunctionCalls\\FibonacciElement\\Main.vm","Main"}};
		Translate(vmfFile,sizeof(vmfFile)/sizeof(VMFile),"OutPut2\\FunctionCalls\\FibonacciElement\\FibonacciElement.asm");
	}

	{
		VMFile vmfFile[]={
			{"OutPut2\\FunctionCalls\\NestedCall\\Sys.vm","Sys"}};
		Translate(vmfFile,sizeof(vmfFile)/sizeof(VMFile),"OutPut2\\FunctionCalls\\NestedCall\\NestedCall.asm");
	}
	{
		VMFile vmfFile[]={
			{"OutPut2\\FunctionCalls\\SimpleFunction\\SimpleFunction.vm","SimpleFunction"}};
		Translate(vmfFile,sizeof(vmfFile)/sizeof(VMFile),"OutPut2\\FunctionCalls\\SimpleFunction\\SimpleFunction.asm");
	}
	{
		VMFile vmfFile[]={
			{"OutPut2\\FunctionCalls\\StaticsTest\\Class1.vm","Class1"},
			{"OutPut2\\FunctionCalls\\StaticsTest\\Class2.vm","Class2"},
			{"OutPut2\\FunctionCalls\\StaticsTest\\Sys.vm","Sys"}};
		Translate(vmfFile,sizeof(vmfFile)/sizeof(VMFile),"OutPut2\\FunctionCalls\\StaticsTest\\StaticsTest.asm");
	}

	{
		VMFile vmfFile[]={
			{"OutPut2\\ProgramFlow\\BasicLoop\\BasicLoop.vm","BasicLoop"}};
		Translate(vmfFile,sizeof(vmfFile)/sizeof(VMFile),"OutPut2\\ProgramFlow\\BasicLoop\\BasicLoop.asm");
	}
	{
		VMFile vmfFile[]={
			{"OutPut2\\ProgramFlow\\FibonacciSeries\\FibonacciSeries.vm","FibonacciSeries"}};
		Translate(vmfFile,sizeof(vmfFile)/sizeof(VMFile),"OutPut2\\ProgramFlow\\FibonacciSeries\\FibonacciSeries.asm");
	}

	{
		VMFile vmfFile[]={
			{"Test.vm","Test"}};
		Translate(vmfFile,sizeof(vmfFile)/sizeof(VMFile),"Test.asm");
	}

}
//*/