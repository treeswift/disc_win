/*---------------------------------------------------------------------------*/
// DisC Decompilation Wizard
// written by
// Satish Kumar S
// satish@miel.mot.com
//
// Copyright 1999-2001 Satish Kumar S
//
// Permission is granted to use this software for research purposes as
// long as this notice stays attached to this software.
/*---------------------------------------------------------------------------*/
#ifndef _DISASM_H		// Disasm.h
#define _DISASM_H

#include "Global.h"
#include "MyString.h"
#include "InstInfo.h"

extern char Regstr[2][8][3];

class Disasm
{
    int Opcode;		// Includes both actual opcode + D + W fields.
    int D,W;
    int Mod,Reg,RM;
    int Len;
    int SRegs;		// Segment registers?
    int FirstOperand;	// Only first operand?
    int SecondOperand;	// Only second operand?

    char *Ip;		// Temporary pointer for sync between procedures.
    Byte FAR *CodePtr;	// Points to the code to be displayed.
    Word regIP;		// The contents of IP for the next instruction.

    String &GetOperands();
    String &Operand1(int Reg,int W,int SRegs = FALSE);
    String &Operand2(int Mod,int RM,int W = 0,int Offset = 0);
    String &Immediate(int Offset = 0);
    String &ImmediateByte(int Offset = 0);
    String &ImmediateWord(int Offset = 0);
    String &ImmediateSByte(int Offset = 0);
    String &ImmediateSWord(int Offset = 0);
    Byte    GetImmediateByte(int Offset = 0);
    Word    GetImmediateWord(int Offset = 0);
    SByte   GetImmediateSByte(int Offset = 0);
    Sword   GetImmediateSWord(int Offset = 0);
    Word    GetImmediate(int Offset = 0);

public:
    InstInfo CurInst;
    Disasm();
    String GetAsm(Byte FAR *Insts,int &InstrLength);
    void TraceInst(Byte FAR *Insts,int &InstrLength);
    Word DispPage(int NumLines);
    void UpdateCodePtr(Byte FAR *Ptr);
    void SetCodePtr(Byte FAR *Ptr);
    void SetregIP(Word IP);
    Dword GetProcSize(Byte FAR *ProcBegin);
};

#endif				// Disasm.h
