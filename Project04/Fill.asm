// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Fill.asm

// Runs an infinite loop that listens to the keyboard input.
// When a key is pressed (any key), the program blackens the screen,
// i.e. writes "black" in every pixel;
// the screen should remain fully black as long as the key is pressed. 
// When no key is pressed, the program clears the screen, i.e. writes
// "white" in every pixel;
// the screen should remain fully clear as long as no key is pressed.

// Put your code here.

//R0->Key State
//R1->Loop
//R2->Fill in
//R3->Space(Pointer,Last Key State)

@0
M=-1

(Input)
@0
D=M
@3
M=D
@KBD
D=M
@0
M=D
@3
D=M-D
@Input
D;JEQ

@0
D=M
@White
D;JEQ

(Black)
@2
M=-1
@FILL
0;JMP

(White)
@2
M=0

(FILL)
@1
M=0
(Loop)
@1
D=M
@SCREEN
D=A+D
@3
M=D
@2
D=M
@3
A=M
M=D

@1
MD=M+1
@8192
D=D-A
@Loop
D;JNE
@Input
0;JMP