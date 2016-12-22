'''
def GetData(File):
    Line=File.readline()
    Keyword=""
    Data=""
    while (len(Line) and not (len(Keyword) and len(Data))):
        First=Line.find("<")
        Last=Line.find(">")
        Keyword=Line[First+1:Last]
        First=Line.rfind("<")
        Data=Line[Last+1:First]
        while (len(Keyword) and Keyword[0]==' '):
            Keyword=Keyword[1:]
        while (len(Keyword) and Keyword[-1]==' '):
            Keyword=Keyword[:-1]
        while (len(Data) and Data[0]==' '):
            Data=Data[1:]
        while (len(Data) and Data[-1]==' '):
            Data=Data[:-1]
        if (not (len(Keyword) and len(Data))):
            Line=File.readline()
    if (len(Keyword) and len(Data)):
        return (Keyword,Data)
    else:
        return 0;
'''

def GetData(File):
    Line=File.readline()
    Keyword=""
    Data=""
    while (len(Line) and not (len(Keyword) and len(Data))):
        First=Line.find("<")
        Last=Line.find(">")
        Keyword=Line[First+1:Last]
        First=Line.rfind("<")
        Data=Line[Last+2:First-1]
        if (not (len(Keyword) and len(Data))):
            Line=File.readline()
    if (len(Keyword) and len(Data)):
        return (Keyword,Data)
    else:
        return 0;
def Block(Out,Count):
    for i in range(0,Count):
        Out.write("  ")
def Output(Out,Data,Count):
    Block(Out,Count)
    Out.write("<%s> %s </%s>\n"%(Data[0],Data[1],Data[0]))


#FunctionStart:1
#Class:2
#Function:4
#Var:8
#ExpressionList:16
#Expression:32
#IgnoreBracket:64

#Value:1
#FunctionStart:2
#FunctionBody:4
#IgnoreBracket:8
#PendingTerm:16

def Settle(In,Out,Count,State=0):
    print "start------"
    State=State
    Data=GetData(In)
    PrivateState=0
    State=State&~32
    if (State&64):
        PrivateState=PrivateState|8
        State=State-64
    if (State&1):
        PrivateState=PrivateState|2
        State=State-1
    while (isinstance(Data,tuple)):
        print "Data:%s"%(Data[1])
        Read=False
        if (PrivateState&2 and Data!=("keyword","var")):
            Block(Out,Count)
            Out.write("<statements>\n")
            Count=Count+1
            PrivateState=PrivateState^6
            print "Private:%d------------------------\nCount:%d"%(PrivateState,Count)
        
        if (Data[0]=="keyword"):
            if (Data[1]=="class"):
                Block(Out,Count)
                Out.write("<class>\n")

                Output(Out,Data,Count+1)
                Data=Settle(In,Out,Count+1,State)
                if (Data!=("symbol","}")):
                    print "Error!"
                    exit(1)
                Out.write("</class>\n")
            elif (Data[1]=="method" or Data[1]=="function" or Data[1]=="constructor"):
                Block(Out,Count)
                Out.write("<subroutineDec>\n")
                Output(Out,Data,Count+1)
                
                Data=Settle(In,Out,Count+1,State)
                if (Data!=("symbol","}")):
                    print "Error!"
                    exit(1)
                Block(Out,Count)
                Out.write("</subroutineDec>\n")
            elif (Data[1]=="var"):
                Block(Out,Count)
                Out.write("<varDec>\n")
                Output(Out,Data,Count+1)
                
                Data=Settle(In,Out,Count+1,State)
                if (Data!=("symbol",";")):
                    print "Error!"
                    exit(1)
                Output(Out,Data,Count+1)
                Block(Out,Count)
                Out.write("</varDec>\n")
            elif (Data[1]=="static" or Data[1]=="field"):
                Block(Out,Count)
                Out.write("<classVarDec>\n")
                Output(Out,Data,Count+1)
                
                Data=Settle(In,Out,Count+1,State)
                if (Data!=("symbol",";")):
                    print "Error!"
                    exit(1)
                Output(Out,Data,Count+1)
                Block(Out,Count)
                Out.write("</classVarDec>\n")
            elif (Data[1]=="let" ):
                Block(Out,Count)
                Out.write("<letStatement>\n")
                Output(Out,Data,Count+1)
                
                Data=Settle(In,Out,Count+1,State)
                if (Data!=("symbol",";")):
                    print "Error!"
                    exit(1)
                Output(Out,Data,Count+1)
                Block(Out,Count)
                Out.write("</letStatement>\n")
            elif (Data[1]=="do"):
                Block(Out,Count)
                Out.write("<doStatement>\n")
                Output(Out,Data,Count+1)
                
                Data=Settle(In,Out,Count+1,State)
                if (Data!=("symbol",";")):
                    print "Error!"
                    exit(1)
                Output(Out,Data,Count+1)
                Block(Out,Count)
                Out.write("</doStatement>\n")
            elif (Data[1]=="if"):
                Block(Out,Count)
                Out.write("<ifStatement>\n")
                Output(Out,Data,Count+1)
                
                Data=Settle(In,Out,Count+1,State|64)
                if (Data!=("symbol","}")):
                    print "Error!"
                    exit(1)
                Data=GetData(In)
                if (len(Data) and Data[0]=="keyword" and Data[1]=="else"):
                    Output(Out,Data,Count+1)
                    Data=Settle(In,Out,Count+1,State)
                    if (Data!=("symbol","}")):
                        print "Error!"
                        exit(1)
                else:
                    Read=True;
                Block(Out,Count)
                Out.write("</ifStatement>\n")
            elif (Data[1]=="while"):
                Block(Out,Count)
                Out.write("<whileStatement>\n")
                Output(Out,Data,Count+1)
                
                Data=Settle(In,Out,Count+1,State|64)
                if (Data!=("symbol","}")):
                    print "Error!"
                    exit(1)
                Block(Out,Count)
                Out.write("</whileStatement>\n")
            elif (Data[1]=="return"):
                Block(Out,Count)
                Out.write("<returnStatement>\n")
                Output(Out,Data,Count+1)
                
                Data=Settle(In,Out,Count+1,State|16)
                if (Data!=("symbol",";")):
                    print "Error!"
                    exit(1)
                Output(Out,Data,Count+1)
                Block(Out,Count)
                Out.write("</returnStatement>\n")
            elif (State&16):
                if (not (State&32)):
                    Block(Out,Count)
                    Out.write("<expression>\n")
                    State=State|32;
                    Count=Count+1
                if (not (PrivateState&1)):
                    Block(Out,Count)
                    Out.write("<term>\n")
                    PrivateState=PrivateState|1;
                    Count=Count+1
                Output(Out,Data,Count)
            else:
                Output(Out,Data,Count)
        elif (Data[0]=="integerConstant" or Data[0]=="stringConstant"):
            if (not (State&32)):
                Block(Out,Count)
                Out.write("<expression>\n")
                State=State|32;
                Count=Count+1
            if (not (PrivateState&1)):
                Block(Out,Count)
                Out.write("<term>\n")
                PrivateState=PrivateState|1;
                Count=Count+1
            Output(Out,Data,Count)
        elif (Data[0]=="identifier"):
            if (State&16):
                if (not (State&32)):
                    Block(Out,Count)
                    Out.write("<expression>\n")
                    State=State|32;
                    Count=Count+1
                if (not (PrivateState&1)):
                    Block(Out,Count)
                    Out.write("<term>\n")
                    PrivateState=PrivateState|1;
                    Count=Count+1
                Output(Out,Data,Count)
            else:
                Output(Out,Data,Count)
                PrivateState=PrivateState|1
        elif (Data[0]=="symbol"):
            if (Data[1]=="("):
                if (State&4):
                    if (not ((PrivateState&8)or (PrivateState&1))):
                        if (not (State&32)):
                            Block(Out,Count)
                            Out.write("<expression>\n")
                            State=State|32;
                            Count=Count+1
                        Block(Out,Count)
                        Out.write("<term>\n")
                        Count=Count+1
                    
                    Output(Out,Data,Count)
                    if (PrivateState&1):
                        Block(Out,Count)
                        Out.write("<expressionList>\n")
                        Count=Count+1

                    Data=Settle(In,Out,Count,State|16)
                    if (Data!=("symbol",")")):
                        print "Error!"
                        exit(1)
                    if (PrivateState&1):
                        Count=Count-1
                        Block(Out,Count)
                        Out.write("</expressionList>\n")
                    Output(Out,Data,Count)

                    if (not ((PrivateState&1) or (PrivateState&8))):
                        Count=Count-1
                        Block(Out,Count)
                        Out.write("</term>\n")
                else:
                    Output(Out,Data,Count)
                    Block(Out,Count)
                    Out.write("<parameterList>\n")

                    Back=Data[1]
                    Data=Settle(In,Out,Count+1,State)
                    if (Data!=("symbol",")")):
                        print "Error!"
                        exit(1)
                    Block(Out,Count)
                    Out.write("</parameterList>\n")
                    Output(Out,Data,Count)
            elif (Data[1]=="["):
                if (PrivateState&1):
                    Output(Out,Data,Count)

                    Data=Settle(In,Out,Count,State|16)
                    if (Data!=("symbol","]")):
                        print "Error!"
                        exit(1)
                    Output(Out,Data,Count)
                else:
                    print "Error!"
                    exit(1)
            elif (Data[1]=="{"):
                if (State&4):
                    Output(Out,Data,Count)
                    Block(Out,Count)
                    Out.write("<statements>\n")

                    Data=Settle(In,Out,Count+1,State)
                    if (Data!=("symbol","}")):
                        print "Error!"
                        exit(1)
                    Block(Out,Count)
                    Out.write("</statements>\n")
                    Output(Out,Data,Count)
                elif (State&2):
                    Block(Out,Count)
                    Out.write("<subroutineBody>\n")
                    Output(Out,Data,Count+1)

                    Data=Settle(In,Out,Count+1,State|4|1)
                    if (Data!=("symbol","}")):
                        print "Error!"
                        exit(1)
                    if (PrivateState&4):
                        Block(Out,Count-1)
                        Out.write("</statements>\n")
                        PrivateState=PrivateState-4
                    
                    Output(Out,Data,Count+1)
                    Block(Out,Count)
                    Out.write("</subroutineBody>\n")
                else:
                    Output(Out,Data,Count)
                    Data=Settle(In,Out,Count,State|2)
                    if (Data!=("symbol","}")):
                        print "Error!"
                        exit(1)
                    Output(Out,Data,Count)
                return Data
            elif (Data[1]==")" or Data[1]=="]" or Data[1]=="}" or Data[1]==";"):
                if (State&16):
                    if (PrivateState&1):
                        Count=Count-1
                        Block(Out,Count)
                        Out.write("</term>\n")
                        PrivateState=PrivateState-1
                    if (PrivateState&16):
                        Count=Count-1
                        Block(Out,Count)
                        Out.write("</term>\n")
                        PrivateState=PrivateState-16
                    if (State&32):
                        Count=Count-1
                        Block(Out,Count)
                        Out.write("</expression>\n")
                        State=State-32
                if (PrivateState&4):
                    Block(Out,Count-1)
                    Out.write("</statements>\n")
                    PrivateState=PrivateState-1
                return Data
            elif (Data[1]==","):
                if (State&16):
                    if (PrivateState&1):
                        Count=Count-1
                        Block(Out,Count)
                        Out.write("</term>\n")
                        PrivateState=PrivateState-1
                    if (PrivateState&16):
                        Count=Count-1
                        Block(Out,Count)
                        Out.write("</term>\n")
                        PrivateState=PrivateState-16
                    if (State&32):
                        Count=Count-1
                        Block(Out,Count)
                        Out.write("</expression>\n")
                        State=State-32
                    Output(Out,Data,Count)
                else:
                    Output(Out,Data,Count)
            elif (Data[1]=="."):
                Output(Out,Data,Count)
            elif (Data[1]=="=" and not (State&16)):
                Output(Out,Data,Count)

                Data=Settle(In,Out,Count,State|16)
                if (Data!=("symbol",";")):
                    print "Error!"
                    exit(1)
                return Data
            else:
                if (not (State&32)):
                    Block(Out,Count)
                    Out.write("<expression>\n")
                    State=State|32;
                    Count=Count+1
                    if (PrivateState&1):
                        PrivateState=PrivateState-1
                    PrivateState=PrivateState|16
                    Block(Out,Count)
                    Out.write("<term>\n")
                    Count=Count+1
                else:
                    if (PrivateState&1):
                        Count=Count-1
                        Block(Out,Count)
                        Out.write("</term>\n")
                        PrivateState=PrivateState-1
                    if (PrivateState&16):
                        Count=Count-1
                        Block(Out,Count)
                        Out.write("</term>\n")
                        PrivateState=PrivateState-16
                Output(Out,Data,Count)
        if (not Read):
            Data=GetData(In)

def Convert(Source,Destination):
    File=open(Source,"r")
    OutFile=open(Destination,"w")
    Settle(File,OutFile,0)
    OutFile.close()
    File.close()

Convert("ArrayTest//MainT.xml","ArrayTest//Main.xml")
Convert("ExpressionLessSquare//MainT.xml","ExpressionLessSquare//Main.xml")
Convert("ExpressionLessSquare//SquareT.xml","ExpressionLessSquare//Square.xml")
Convert("ExpressionLessSquare//SquareGameT.xml","ExpressionLessSquare//SquareGame.xml")
Convert("Square//MainT.xml","Square//Main.xml")
Convert("Square//SquareT.xml","Square//Square.xml")
Convert("Square//SquareGameT.xml","Square//SquareGame.xml")
