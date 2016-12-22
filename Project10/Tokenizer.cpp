#include <iostream>
#include <fstream>
#include <map>
#include <set>
#include <MsXml2.h>
#include <stdio.h>

using namespace std;

void Block(ostream& osOut,unsigned long ulCount)
{
	for (unsigned long ulLoop=0;ulLoop<ulCount;++ulLoop)
	{
		osOut<<"    ";
	}
}

void Parse(istream& isIn,ostream& osOut,set<string>& ssKeyword,map<char,string>& mMap)
{
	char acSpace[1000];
	set<string> sKeyWord;
	unsigned long ulCount=0;
	unsigned long ulLoop;
	osOut<<"<tokens>"<<endl;
	while (!(isIn>>acSpace[0]).eof())
	{
		acSpace[1]='/';
		Block(osOut,ulCount);
		while (acSpace[0]=='/'&&strchr("/*",acSpace[1]))
		{
			acSpace[1]=isIn.get();
			switch (acSpace[1])
			{
			case '/':
				while (!strchr("\r\n",isIn.get()));
				while (strchr("\r\n \t",acSpace[0]=isIn.get()));
				break;
			case '*':
				acSpace[0]=isIn.get();
				acSpace[1]=isIn.get();
				while (acSpace[0]!='*'||acSpace[1]!='/')
				{
					acSpace[0]=acSpace[1];
					acSpace[1]=isIn.get();
				}
				while (strchr("\r\n \t",acSpace[0]=isIn.get()));
				break;
			default:
				isIn.putback(acSpace[1]);
			};
		}
		if (acSpace[0]>='0'&&acSpace[0]<='9')
		{
			unsigned __int16 ui16Space;
			isIn.putback(acSpace[0]);
			isIn>>ui16Space;
			osOut<<"<integerConstant> "<<ui16Space<<" </integerConstant>"<<endl;
		}
		else if ((acSpace[0]&~0x20)>='A'&&(acSpace[0]&~0x20)<='Z')
		{
			ulLoop=0;
			do
			{
				acSpace[++ulLoop]=isIn.get();
			} while (acSpace[ulLoop]!=EOF&&!isIn.eof()&&(acSpace[ulLoop]&~0x20)>='A'&&(acSpace[ulLoop]&~0x20)<='Z'||acSpace[ulLoop]>='0'&&acSpace[ulLoop]<='9');
			isIn.putback(acSpace[ulLoop]);
			acSpace[ulLoop]=0;
			if (ssKeyword.find(acSpace)==ssKeyword.end())
			{
				osOut<<"<identifier> "<<acSpace<<" </identifier>"<<endl;
			}
			else
			{
				osOut<<"<keyword> "<<acSpace<<" </keyword>"<<endl;
			}
		}
		else if (acSpace[0]=='\"')
		{
			ulLoop=~0;
			do
			{
				acSpace[++ulLoop]=isIn.get();
			} while (acSpace[ulLoop]!=EOF&&!isIn.eof()&&acSpace[ulLoop]!='\"');
			acSpace[ulLoop]=0;
			osOut<<"<stringConstant> "<<acSpace<<" </stringConstant>"<<endl;
		}
		else
		{
				osOut<<"<symbol> ";
				if (mMap.find(acSpace[0])==mMap.end())
					osOut<<acSpace[0];
				else
					osOut<<mMap[acSpace[0]].c_str();
				osOut<<" </symbol>"<<endl;
		}
	}
	osOut<<"</tokens>"<<endl;
}

int main(int argc,char* argv[],char* envp[])
{
	if (argc>1)
	{
		string sSource=argv[1];
		string sDestination;
		size_t sDot=sSource.find_last_of('.');
		size_t sDash=sSource.find_last_of('\\');
		if (sDot>sDash)
			sDestination=sSource.substr(0,sDot)+"T.xml";
		else
			sDestination=sSource+"T.xml";
		set<string> sSet;
		map<char,string> mMap;
		{
			char const* acKeyWord[]={"class","constructor","function","method","field","static","var","int","char","boolean","void","true","false","null","this","let","do","if","else","while","return"};
			for (unsigned long ulLoop=0;ulLoop<sizeof(acKeyWord)/sizeof(char const*);++ulLoop)
			{
				sSet.insert(acKeyWord[ulLoop]);
			}
		}
		{
			mMap['<']="&lt;";
			mMap['>']="&gt;";
			mMap['&']="&amp;";
		}

		Parse(ifstream(sSource),ofstream(sDestination),sSet,mMap);
		system("pause");
	}
}