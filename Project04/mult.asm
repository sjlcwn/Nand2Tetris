// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Mult.asm

// Multiplies R0 and R1 and stores the result in R2.
// (R0, R1, R2 refer to RAM[0], RAM[1], and RAM[2], respectively.)

// Put your code here.
@2
M=0
@1
D=M
@3
M=D	//Ram[3]=Ram[1]*pow(2,n-1)
@4
MD=1	//Ram[4]=2<<n+1
@Start
0;JMP
(Add)
@3
D=M
@2
M=D+M
(Next)
@3
D=M
M=D+M
@4
D=M
MD=D+M
(Start)
@0
D=D&M
@Add
D;JNE
@3
D=M
@Next
D;JNE
(Loop)
@Loop
0;JMP