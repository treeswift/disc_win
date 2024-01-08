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
#ifndef _INSTINFO_H		// InstInfo.h
#define _INSTINFO_H

#include "Global.h"
#include "MyString.h"
#include "Variable.h"
class InstInfo
{
public:
    int Instr;			// Operation.

    int operSize1;
    int Operand1;			// Type of operand.
    Word Data11;			// Operand 1. (Value depends on the Type).
    Sword Data12;			// Used for indexing addr. modes.

    int operSize2;
    int Operand2;
    Word Data21;
    Sword Data22;

    void Clear();
    void SwapOperands();
    String GetOperand(int oper,String *Regs,int getName=1);
    inline String GetOperand1(String *Regs,int getName=1)
    { return GetOperand(1,Regs,getName); }
    inline String GetOperand2(String *Regs,int getName=1)
    { return GetOperand(2,Regs,getName); }
    Variable &GetVariable(Variable &t,int whichVar);
    Variable &SetVariable(Variable &t,int whichVar);
};
// Values for the Registers in the same order they appear in the machine code
enum
{
    REG_AL=0, REG_CL, REG_DL, REG_BL, REG_AH, REG_CH, REG_DH, REG_BH,
    REG_AX,   REG_CX, REG_DX, REG_BX, REG_SP, REG_BP, REG_SI, REG_DI,
    REG_ES,   REG_CS, REG_SS, REG_DS
};

// Values for Instr.
enum
{
    UNKNOWN=-1, PUSH, POP, CALL, RET,
    LEA, CBW, CWD, INC, DEC, JMP,
    MOV, ADD, SUB, AND, OR, XOR,		// Order is very important!!
    ADC, SBB, CMP,
    TEST, NOT, NEG, MUL, IMUL, DIV, IDIV,
    ROL, ROR, RCL, RCR, SHL, SHR, SAL, SAR,
    JA, JAE, JB, JBE, JZ, JNZ, JG, JGE, JL, JLE,
    JO, JNO, JS, JNS, JPE, JPO,
    LOOPZ, LOOPNZ, LOOP, JCXZ,
    SEGMENT, LES
};
// Values for Operands 1&2.
enum
{
    OPER_UNKNOWN=-1,
    REG_DIRECT,REG_INDIRECT,INDEXED_BYTE,INDEXED_WORD,
    MEMORY,IMMEDIATE
};
// Values for Data11/21 if Operand1/2=INDEXED_xxxx mode
enum
{
    INDX_BX_SI, INDX_BX_DI, INDX_BP_SI, INDX_BP_DI,
    INDX_SI,    INDX_DI,    INDX_BP,    INDX_BX
};

#endif				// InstInfo.h
