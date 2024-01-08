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
#include <conio.h>
#include <stdio.h>
#include <dos.h>
#include "Disasm.h"

#define AddOffset(str) { CurInst.Data11 = (Word)(regIP + Len + (int)Ip[1]); \
			 str += CurInst.Data11; }
#define AddOnlyOffset() CurInst.Data11 = (Word)(regIP + Len + (int)Ip[1])

#define Register(W,R) (W*8 + R)
#define SegmentReg(R) (16 + R)

char Regstr[2][8][3]	= {"AL","CL","DL","BL","AH","CH","DH","BH",\
				"AX","CX","DX","BX","SP","BP","SI","DI"};
char SRegstr[][3]	= {"ES","CS","SS","DS"};
char Effaddrstr[8][6]	= {"BX+SI","BX+DI","BP+SI","BP+DI","SI","DI","BP","BX"};
char ArithOperstr[][5]	= {"ADD ","OR ","ADC ","SBB ","AND ","SUB ","XOR ","CMP "};
char SecArithOperstr[][6] = {"TEST ","","NOT ","NEG ","MUL ","IMUL ","DIV ","IDIV "};
char LogOperstr[][5]	= {"ROL ","ROR ","RCL ","RCR ","SHL ","SHR ","SAL ","SAR "};
char ByteorWordPtr[][10]= {"BYTE PTR ","WORD PTR "};
int ArithOpers[]	= {ADD, OR, ADC, SBB, AND, SUB, XOR, CMP };
int SecArithOpers[]	= {TEST, UNKNOWN, NOT, NEG, MUL, IMUL, DIV, IDIV };
int LogOpers[]		= {ROL, ROR, RCL, RCR, SHL, SHR, SAL, SAR };

Disasm::Disasm()
{
  CodePtr = NULL;
  regIP = 0;
}
void Disasm::SetCodePtr(Byte FAR *Ptr)
{
  regIP = 0;
  CodePtr = Ptr;
}
void Disasm::SetregIP(Word IP)
{
  regIP = IP;
}
void Disasm::UpdateCodePtr(Byte FAR *Ptr)
{
  regIP = (Ptr - CodePtr);	// Update IP for new position.
  CodePtr = Ptr;
}
Byte Disasm::GetImmediateByte(int Offset /* = 0 */)
{
  return (*(Byte FAR *)(Ip + Offset));
}
Word Disasm::GetImmediateWord(int Offset /* = 0 */)
{
  return (*(Word FAR *)(Ip + Offset));
}
SByte Disasm::GetImmediateSByte(int Offset /* = 0 */)
{
  return (*(SByte FAR *)(Ip + Offset));
}
Sword Disasm::GetImmediateSWord(int Offset /* = 0 */)
{
  return (*(Sword FAR *)(Ip + Offset));
}
String &Disasm::ImmediateByte(int Offset /* = 0 */)
{
  static String tmp;
  tmp=(*(Byte FAR *)(Ip+Offset));
  return tmp;
}
String &Disasm::ImmediateWord(int Offset /* = 0 */)
{
  static String tmp;
  tmp=(*(Word FAR *)(Ip+Offset));
  return tmp;
}
String &Disasm::Immediate(int Offset /* = 0 */)
{
  Len = Offset + 1 + W;
  if (W) return ImmediateWord(Offset);
  else return ImmediateByte(Offset);
}
Word Disasm::GetImmediate(int Offset /* = 0 */)
{
  Len = Offset + 1 + W;
  if (W) return GetImmediateWord(Offset);
  else return (Word)GetImmediateByte(Offset);
}
String &Disasm::ImmediateSByte(int Offset /* = 0 */)
{
  int val = (int)*(char FAR *)(Ip+Offset);
  static String tmp;
  tmp = (val<0)?"-":"+";
  tmp += (val<0)?(Byte)(256-val):(Byte)val;
  return tmp;
}
String &Disasm::ImmediateSWord(int Offset /* = 0 */)
{
  int val = (int)*(int FAR *)(Ip+Offset);
  static String tmp;
  tmp = (val<0)?"-":"+";
  tmp += (val<0)?(Word)(65536-val):(Word)val;
  return tmp;
}
String &Disasm::Operand1(int Reg,int W,int SRegs /* = FALSE */)
{
  static String operand;
  CurInst.Operand1 = REG_DIRECT;
  CurInst.Data11 = (SRegs)?SegmentReg(Reg):Register(W,Reg);
  operand = (SRegs) ? SRegstr[Reg] : Regstr[W][Reg];
  return operand;
}
String &Disasm::Operand2(int Mod,int RM,int W /* = 0 */,int Offset /* = 0 */)
{
  static String operand;
  operand="[";
  String Nul;
  switch(Mod)
  {
    case 0 : if (RM == 6)
	     {
	       CurInst.Operand2 = MEMORY;
	       CurInst.Data21 = GetImmediateWord(Offset);
	       operand += ImmediateWord(Offset);
	       Len += 2;
	     }
	     else
	     {
	       CurInst.Operand2 = REG_INDIRECT;
	       CurInst.Data21 = RM;
	       operand += Effaddrstr[RM];
	       break;
	     }
	     break;
    case 1 : CurInst.Operand2 = INDEXED_BYTE;
	     CurInst.Data21 = RM;
	     CurInst.Data22 = (Sword)GetImmediateSByte(Offset);
	     operand += Nul + Effaddrstr[RM] + ImmediateSByte(Offset);
	     Len++;
	     break;
    case 2 : CurInst.Operand2 = INDEXED_WORD;
	     CurInst.Data21 = RM;
	     CurInst.Data22 = (Sword)GetImmediateSWord(Offset);
	     operand += Nul + Effaddrstr[RM] + ImmediateSWord(Offset);
	     Len+=2;
	     break;
    case 3 : CurInst.Operand2 = REG_DIRECT;
	     CurInst.Data21 = Register(W,RM);
	     operand = Regstr[W][RM];
	     break;
  }
  if (Mod!=3) operand += "]";
  return operand;
}
String &Disasm::GetOperands()
{
  static String str;
  if (D==0)
  {
    str = Operand2(Mod,RM,W,2) + "," + Operand1(Reg,W);
    CurInst.SwapOperands();
  }
  else str = Operand1(Reg,W) + "," + Operand2(Mod,RM,W,2);
  return str;
}
void Disasm::TraceInst(Byte FAR *Insts,int &Length)
{
  regIP = (Insts - CodePtr);
  Ip = (char *)Insts;
  Opcode = Insts[0];
  D   = (Opcode >> 1) & 1;
  W   = Opcode & 1;
  Mod = (Insts[1] >> 6) & 3;
  Reg = (Insts[1] >> 3) & 7;
  RM  = Insts[1] & 7;
  int ArithOper = (Opcode >> 3) & 7;

  Len = 1;
  SRegs = FALSE;
  FirstOperand = FALSE;
  SecondOperand = FALSE;

  CurInst.Clear();
  String str;
  switch(Opcode)
  {
    // Mov AL,[mem].  Mov AX,[mem].  Mov [mem],AL.  Mov [mem],AX.
    case 0xa0 : case 0xa1 : case 0xa2 : case 0xa3 :
		CurInst.Instr = MOV;
		if (D)
		{
		  CurInst.Operand2 = REG_DIRECT;
		  CurInst.Data21 = Register(W,0);
		  CurInst.Operand1 = MEMORY;
		  CurInst.Data11 = GetImmediateWord(1);
		}
		else
		{
		  CurInst.Operand1 = REG_DIRECT;
		  CurInst.Data11 = Register(W,0);
		  CurInst.Operand2 = MEMORY;
		  CurInst.Data21 = GetImmediateWord(1);
		}
		Len = 3; break;
    // Mov Addr,Reg8. & Mov Addr,Reg16.
    case 0x88 : case 0x89 :
    // Mov Reg8,Addr. & Mov Reg16,Addr.
    case 0x8a : case 0x8b :
		CurInst.Instr = MOV; Len = 2;
		GetOperands();
		break;
    // Mov Reg8,Immediate8.
    case 0xb0 : case 0xb1 : case 0xb2 : case 0xb3 :
    case 0xb4 : case 0xb5 : case 0xb6 : case 0xb7 :
    // Mov Reg16,Immediate16.
    case 0xb8 : case 0xb9 : case 0xba : case 0xbb :
    case 0xbc : case 0xbd : case 0xbe : case 0xbf :
		Reg = Opcode & 15;
		Len = 2;
		CurInst.Instr = MOV;
		CurInst.Operand1 = REG_DIRECT;
		CurInst.Data11 = Reg;
		CurInst.Operand2 = IMMEDIATE;
		W = (Reg>>3)&1;
		CurInst.operSize1 = CurInst.operSize2 = W;
		CurInst.Data21 = GetImmediate(1);
		break;
    // Mov Addr,Imm.
    case 0xc6 : case 0xc7 :
		Len = 2;
		CurInst.Instr = MOV;
		CurInst.operSize2 = W;
		Operand2(Mod,RM,W,2);
		CurInst.Operand1 = IMMEDIATE;
		CurInst.operSize1 = W;
		CurInst.Data11 = GetImmediate(Len);
		CurInst.SwapOperands();
		break;
    // Mov Addr,SReg.
    case 0x8c : Len = 2;
		CurInst.Instr = MOV;
		if (RM==0)
		{
		  CurInst.Operand1 = REG_DIRECT;
		  CurInst.Data11 = RM;
		}
		else
		{
		  Operand2(Mod,RM,1,2);
		  CurInst.SwapOperands();
		}
		CurInst.Operand2 = REG_DIRECT;
		CurInst.Data21 = SegmentReg(Reg);
		break;
    // Mov Sreg,Addr.
    case 0x8e : Len = 2;
		CurInst.Instr = MOV;
		CurInst.Operand1 = REG_DIRECT;
		CurInst.Data11 = SegmentReg(Reg);
		if (RM==0)
		{
		  CurInst.Operand2 = REG_DIRECT;
		  CurInst.Data21 = RM;
		}
		else
		{
		  Operand2(Mod,RM,1,2);
		}
		break;
    // Ret & Retf
    case 0xc2 : CurInst.Instr = RET;
		CurInst.operSize1=0;
		CurInst.Data11 = GetImmediateWord(1);
		break;
    case 0xc3 : CurInst.Instr = RET;
		CurInst.operSize1=0;
		CurInst.Data11 = 0; break;
    case 0xca : CurInst.Instr = RET;
		CurInst.operSize1=1;
		CurInst.Data11 = GetImmediateWord(1);
		break;
    case 0xcb : CurInst.Instr = RET;
		CurInst.operSize1=0;
		CurInst.Data11 = 0; break;
    // (Inc,Dec,Call,Call FAR,Jmp,Jmp FAR,Push)  Addr.
    case 0xff : Len = 2;
		switch(Reg)
		{
		  case 0 : CurInst.Instr = INC; break;
		  case 1 : CurInst.Instr = DEC; break;
		  case 2 : CurInst.Instr = CALL; CurInst.operSize2=0;
			   CurInst.Data22=FP_SEG(Insts); break;
		  case 3 : CurInst.Instr = CALL; CurInst.operSize2=1; break;
		  case 4 : CurInst.Instr = JMP; CurInst.operSize2=0;
			   CurInst.Data22=FP_SEG(Insts); break;
		  case 5 : CurInst.Instr = JMP; CurInst.operSize2=1; break;
		  case 6 : CurInst.Instr = PUSH; break;
		  case 7 : CurInst.Instr = UNKNOWN;
		}
		Operand2(Mod,RM,1,2);
		CurInst.SwapOperands();
		break;
    // Push Reg.
    case 0x50 : case 0x51 :
    case 0x52 : case 0x53 :
    case 0x54 : case 0x55 :
    case 0x56 : case 0x57 :
		Reg = Opcode & 7; Len = 1;
		CurInst.Instr = PUSH;
		CurInst.Operand1 = REG_DIRECT;
		CurInst.Data11 = Register(1,Reg);
		break;
    // Push SReg.
    case 0x06 : case 0x0e :
    case 0x16 : case 0x1e :
		Reg = (Opcode >> 3) & 3; Len = 1;
		CurInst.Instr = PUSH;
		CurInst.Operand1 = REG_DIRECT;
		CurInst.Data11 = SegmentReg(Reg);
		break;
    // Pop Sreg.
    case 0x07 : case 0x0f :
    case 0x17 : case 0x1f :
		Reg = (Opcode >> 3) & 3; Len = 1;
		CurInst.Instr = POP;
		CurInst.Operand1 = REG_DIRECT;
		CurInst.Data11 = SegmentReg(Reg);
		break;
    // Pop [mem], Pop [reg].
    case 0x8f : CurInst.Instr = POP; Len = 2;
		Operand2(Mod,RM,1,2);
		CurInst.SwapOperands();
		break;
    // Pop Reg.
    case 0x58 : case 0x59 :
    case 0x5a : case 0x5b :
    case 0x5c : case 0x5d :
    case 0x5e : case 0x5f :
		Reg = Opcode & 7; Len = 1;
		CurInst.Instr = POP;
		CurInst.Operand1 = REG_DIRECT;
		CurInst.Data11 = Register(1,Reg);
		break;
    // ArithOper Addr,Imm ( Includes add,sub,adc,sbb,and,or,xor,cmp )
    case 0x80 : case 0x81 : case 0x82 : case 0x83 :
		ArithOper = Reg; Len = 2;
		CurInst.Instr = ArithOpers[ArithOper];
		CurInst.operSize2 = W;
		Operand2(Mod,RM,W,2);
		if (W) W = 1-D;
		CurInst.operSize1 = W;
		CurInst.Operand1 = IMMEDIATE;
		CurInst.Data11 = GetImmediate(Len);
		CurInst.SwapOperands();
		break;
    // ArithOper AL,Imm.
    case 0x04 : case 0x0c : case 0x14 : case 0x1c :
    case 0x24 : case 0x2c : case 0x34 : case 0x3c :
		Len = 2;
		CurInst.Instr = ArithOpers[ArithOper];
		CurInst.Operand1 = REG_DIRECT;
		CurInst.Data11 = Register(0,0);
		CurInst.Operand2 = IMMEDIATE;
		CurInst.Data21 = (Word)GetImmediateByte(1);
		break;
    // ArithOper AX,Imm.
    case 0x05 : case 0x0d : case 0x15 : case 0x1d :
    case 0x25 : case 0x2d : case 0x35 : case 0x3d :
		Len = 3;
		CurInst.Instr = ArithOpers[ArithOper];
		CurInst.Operand1 = REG_DIRECT;
		CurInst.Data11 = Register(1,0);
		CurInst.Operand2 = IMMEDIATE;
		CurInst.Data21 = GetImmediateWord(1);
		break;
    // ArithOper Addr,Addr.
    case 0x00 : case 0x01 : case 0x02 : case 0x03 :
    case 0x08 : case 0x09 : case 0x0a : case 0x0b :
    case 0x10 : case 0x11 : case 0x12 : case 0x13 :
    case 0x18 : case 0x19 : case 0x1a : case 0x1b :
    case 0x20 : case 0x21 : case 0x22 : case 0x23 :
    case 0x28 : case 0x29 : case 0x2a : case 0x2b :
    case 0x30 : case 0x31 : case 0x32 : case 0x33 :
    case 0x38 : case 0x39 : case 0x3a : case 0x3b :
		Len = 2;
		CurInst.Instr = ArithOpers[ArithOper];
		GetOperands();
		break;
    // Inc Reg16.
    case 0x40 : case 0x41 : case 0x42 : case 0x43 :
    case 0x44 : case 0x45 : case 0x46 : case 0x47 :
    // Dec Reg16.
    case 0x48 : case 0x49 : case 0x4a : case 0x4b :
    case 0x4c : case 0x4d : case 0x4e : case 0x4f :
		Reg = Opcode & 7; Len = 1;
		CurInst.Instr = (Opcode & 8) ? DEC : INC;
		CurInst.Operand1 = REG_DIRECT;
		CurInst.Data11 = Register(1,Reg);
		break;
    // Inc Reg8. & Dec Reg8.
    case 0xfe : Opcode = Ip[1];
		Reg = Opcode & 7; Len = 2;
		CurInst.Instr = (Opcode & 8) ? DEC : INC;
		CurInst.Operand1 = REG_DIRECT;
		CurInst.Data11 = Register(0,Reg);
		break;
    // Lea Reg, mem.
    case 0x8d : Len = 2;
		CurInst.Instr = LEA;
		CurInst.Operand1 = REG_DIRECT;
		CurInst.Data11 = Register(W,Reg);
		Operand2(Mod,RM,W,2);
		break;
    case 0x98 : CurInst.Instr = CBW; break;
    case 0x99 : CurInst.Instr = CWD; break;

    // Call Imm.
    case 0xe8 : Len = 3;
		CurInst.Instr = CALL;
		CurInst.operSize1=0;
		CurInst.Operand1=IMMEDIATE;
		CurInst.Data11 = (Word)(regIP + Len + (*(int FAR *)(Ip + 1)));
		CurInst.Data12 = FP_SEG(Insts);
		break;
    // Call FAR Imm.
    case 0x9a : Len = 5;
		CurInst.Instr = CALL;
		CurInst.operSize1=1;
		CurInst.Operand1=IMMEDIATE;
		CurInst.Data11 = GetImmediateWord(1);
		CurInst.Data12 = GetImmediateWord(3);
		break;
    // SecArithOper Addr.
    case 0xf6 : case 0xf7 :
		ArithOper = Reg;
		Reg = RM; Len = 2;
		CurInst.Instr = SecArithOpers[ArithOper];
		CurInst.operSize1 = W;
		CurInst.operSize2 = W;
		if (ArithOper == 0)		// Test Addr,Imm.
		{
		  Operand2(Mod,RM,W,2);
		  CurInst.Operand1 = IMMEDIATE;
		  CurInst.Data11 = GetImmediate(2);
		  CurInst.SwapOperands();
		}
		else				// Others.
		{
		  Operand2(Mod,RM,W,2);
		  CurInst.SwapOperands();
		}
		break;
    // (SHL,SHR,SAL,SAR) mem/reg,1. or mem/reg,cl.
    case 0xd0 : case 0xd1 : case 0xd2 : case 0xd3 :
		str = LogOperstr[Reg];
		CurInst.Instr = LogOpers[Reg];
		if (RM==6)
		{
		  str += ByteorWordPtr[W]; Len = 4;
		  str += "["; str += ImmediateWord(2); str += "]";
		  CurInst.Operand1 = MEMORY;
		  CurInst.Data11 = GetImmediateWord(2);
		}
		else
		{
		  str += Operand1(RM,W); Len = 2;
		}
		if (D)
		{
		  CurInst.Operand2 = REG_DIRECT;
		  CurInst.Data21 = REG_CL;
		}
		else
		{
		  CurInst.Operand2 = IMMEDIATE;
		  CurInst.Data21 = 1;
		}
		break;
    // Test Addr,Addr.
    case 0x84 : case 0x85 :
		str = "TEST "; Len = 2;
		if (D==0) str += Operand1(Reg,W) + "," + Operand2(Mod,RM,W,2);
		else str += Operand2(Mod,RM,W,2) + "," + Operand1(Reg,W);
		break;
    // Test AL,Imm.
    case 0xa8 : str = "TEST AL,"; str += ImmediateByte(1); Len = 2; break;
    // Test AX,Imm.
    case 0xa9 : str = "TEST AX,"; str += ImmediateWord(1); Len = 3; break;
    // Jmp Imm.(2 byte displacement).
    case 0xe9 : CurInst.Instr = JMP; Len = 3;
		CurInst.operSize1=0;
		CurInst.Operand1 = IMMEDIATE;
		CurInst.Data11 = (Word)(regIP + Len + *(int FAR *)(Ip+1));
		break;
    // Jmp FAR Imm.
    case 0xea : CurInst.Instr = JMP; Len = 5;
		CurInst.operSize1=1;
		CurInst.Operand1 = IMMEDIATE;
		CurInst.Data11 = GetImmediateWord(1);
		CurInst.Data21 = GetImmediateWord(3);
		break;
    // Jmp short Imm.(-128 to +127 displacement).
    case 0xeb : CurInst.Instr = JMP;
		CurInst.operSize1=0;
		CurInst.Operand1 = IMMEDIATE;
		Len=2; AddOnlyOffset(); break;
    // Conditional jmps.
    case 0x70 : CurInst.Instr = JO;  Len = 2; AddOnlyOffset(); break;
    case 0x71 : CurInst.Instr = JNO; Len = 2; AddOnlyOffset(); break;
    case 0x72 : CurInst.Instr = JB;  Len = 2; AddOnlyOffset(); break;
    case 0x73 : CurInst.Instr = JAE; Len = 2; AddOnlyOffset(); break;
    case 0x74 : CurInst.Instr = JZ;  Len = 2; AddOnlyOffset(); break;
    case 0x75 : CurInst.Instr = JNZ; Len = 2; AddOnlyOffset(); break;
    case 0x76 : CurInst.Instr = JBE; Len = 2; AddOnlyOffset(); break;
    case 0x77 : CurInst.Instr = JA;  Len = 2; AddOnlyOffset(); break;
    case 0x78 : CurInst.Instr = JS;  Len = 2; AddOnlyOffset(); break;
    case 0x79 : CurInst.Instr = JNS; Len = 2; AddOnlyOffset(); break;
    case 0x7a : CurInst.Instr = JPE; Len = 2; AddOnlyOffset(); break;
    case 0x7b : CurInst.Instr = JPO; Len = 2; AddOnlyOffset(); break;
    case 0x7c : CurInst.Instr = JL;  Len = 2; AddOnlyOffset(); break;
    case 0x7d : CurInst.Instr = JGE; Len = 2; AddOnlyOffset(); break;
    case 0x7e : CurInst.Instr = JLE; Len = 2; AddOnlyOffset(); break;
    case 0x7f : CurInst.Instr = JG;  Len = 2; AddOnlyOffset(); break;

    // Loops.
    case 0xe0 : CurInst.Instr = LOOPNZ; Len = 2; AddOnlyOffset(); break;
    case 0xe1 : CurInst.Instr = LOOPZ;  Len = 2; AddOnlyOffset(); break;
    case 0xe2 : CurInst.Instr = LOOP;   Len = 2; AddOnlyOffset(); break;
    case 0xe3 : CurInst.Instr = JCXZ;   Len = 2; AddOnlyOffset(); break;

    // Segment Overrides.
    case 0x26 : case 0x2e : case 0x36 : case 0x3e :
		CurInst.Instr = SEGMENT;
		CurInst.Data11 = REG_ES + ((Opcode>>3)&3);
		Len = 1;
		break;

    case 0x27 : str = "DAA"; break;
    case 0x2f : str = "DAS"; break;
    case 0x37 : str = "AAA"; break;
    case 0x3f : str = "AAS"; break;

    case 0x90 : str = "NOP"; break;
    case 0x91 : case 0x92 : case 0x93 : case 0x94 :
    case 0x95 : case 0x96 : case 0x97 :
		str = "XCHG ";
		Reg = Opcode & 7;
		str += Operand1(Reg,1) + ",AX";
		break;
    case 0x9b : str = "WAIT"; break;
    case 0x9c : str = "PUSHF"; break;
    case 0x9d : str = "POPF"; break;
    case 0x9e : str = "SAHF"; break;
    case 0x9f : str = "LAHF"; break;

    case 0xa4 : str = "MOVSB"; break;
    case 0xa5 : str = "MOVSW"; break;
    case 0xa6 : str = "CMPSB"; break;
    case 0xa7 : str = "CMPSW"; break;
    case 0xaa : str = "STOSB"; break;
    case 0xab : str = "STOSW"; break;
    case 0xac : str = "LODSB"; break;
    case 0xad : str = "LODSW"; break;
    case 0xae : str = "SCASB"; break;
    case 0xaf : str = "SCASW"; break;

    case 0xcd : str = "INT "; str += ImmediateByte(1); Len = 2; break;
    case 0xce : str = "INTO"; break;
    case 0xcf : str = "IRET"; break;

    case 0xd5 : str = "AAD"; break;
    case 0xd7 : str = "XLAT"; break;
    case 0xda : str = "AAM"; break;

    case 0xf0 : str = "LOCK"; break;
    case 0xf2 : str = "REPNZ"; break;
    case 0xf3 : str = "REPZ"; break;
    case 0xf4 : str = "HLT"; break;
    case 0xf5 : str = "CMC"; break;
    case 0xf8 : str = "CLC"; break;
    case 0xf9 : str = "STC"; break;
    case 0xfa : str = "CLI"; break;
    case 0xfb : str = "STI"; break;
    case 0xfc : str = "CLD"; break;
    case 0xfd : str = "STD"; break;

    // Xchg Addr,Addr.
    case 0x86 :
    case 0x87 : str = "XCHG "; Len = 2; str += GetOperands();
		break;
    // In.
    case 0xec : str = "IN AL,DX"; break;
    case 0xed : str = "IN AX,DX"; break;
    case 0xe4 : case 0xe5 :
		str = "IN "; str += Operand1(Reg,W); str += ",";
		str += ImmediateByte(1); Len = 3;
		break;
    // Out.
    case 0xee : str = "OUT DX,AL"; break;
    case 0xef : str = "OUT DX,AX"; break;
    case 0xe6 : case 0xe7 :
		str = "OUT "; str += Operand1(Reg,W); str += ",";
		str += ImmediateByte(1); Len = 3;
		break;
    // les Reg, mem.
    case 0xc4 : W = 1; D = 1-D; Len = 2;
		str = "LES "; str += GetOperands();
		CurInst.Instr=LES;
		break;
    // lds Reg, mem.
    case 0xc5 : W = 1; D = 1-D; Len = 2;
		str = "LDS "; str += GetOperands();
		break;
    // Unknown.
    default   : str = "????";
  }
  Length = Len;
  regIP += Len;
}
String Disasm::GetAsm(Byte FAR *Insts,int &Length)
{
  regIP = (Insts - CodePtr);
  Ip = (char *)Insts;
  Opcode = Insts[0];
  D   = (Opcode >> 1) & 1;
  W   = Opcode & 1;
  Mod = (Insts[1] >> 6) & 3;
  Reg = (Insts[1] >> 3) & 7;
  RM  = Insts[1] & 7;
  int ArithOper = (Opcode >> 3) & 7;

  Len = 1;
  SRegs = FALSE;
  FirstOperand = FALSE;
  SecondOperand = FALSE;

  String str;
  switch(Opcode)
  {
    // Mov AL,[mem].
    case 0xa0 : str = "MOV AL,["; str += ImmediateWord(1) + "]";
		Len = 3; break;
    // Mov AX,[mem].
    case 0xa1 : str = "MOV AX,["; str += ImmediateWord(1) + "]";
		Len = 3; break;
    // Mov [mem],AL.
    case 0xa2 : str = "MOV ["; str += ImmediateWord(1) + "],AL";
		Len = 3; break;
    // Mov [mem],AX.
    case 0xa3 : str = "MOV ["; str += ImmediateWord(1) + "],AX";
		Len = 3; break;
    // Mov Addr,Reg8. & Mov Addr,Reg16.
    case 0x88 : case 0x89 :
    // Mov Reg8,Addr. & Mov Reg16,Addr.
    case 0x8a : case 0x8b :
		str = "MOV "; Len = 2;
		str += GetOperands();
		break;
    // Mov Addr,SReg.
    case 0x8c : str = "MOV "; Len = 2;
		if (RM==0) str += Operand1(RM,1);
		else str += Operand2(Mod,RM,1,2);
		str += ",";
		str += Operand1(Reg,1,TRUE);
		break;
    // Mov Sreg,Addr.
    case 0x8e : str = "MOV "; Len = 2;
		str += Operand1(Reg,1,TRUE) + ",";
		if (RM==0) str += Operand1(RM,1);
		else str += Operand2(Mod,RM,1,2);
		break;
    // Mov Reg8,Immediate8.
    case 0xb0 : case 0xb1 :
    case 0xb2 : case 0xb3 :
    case 0xb4 : case 0xb5 :
    case 0xb6 : case 0xb7 :
		str = "MOV "; Len = 2;
		Reg = Opcode & 7;
		str += Operand1(Reg,0) + "," + ImmediateByte(1); break;
    // Mov Reg16,Immediate16.
    case 0xb8 : case 0xb9 :
    case 0xba : case 0xbb :
    case 0xbc : case 0xbd :
    case 0xbe : case 0xbf :
		str = "MOV "; Len = 3;
		Reg = Opcode & 7;
		str += Operand1(Reg,1) + "," + ImmediateWord(1); break;
    // Mov Addr,Imm.
    case 0xc6 : case 0xc7 :
		str = "MOV "; Len = 2;
		str += ByteorWordPtr[W];
		str += Operand2(Mod,RM,W,2) + ",";
		str += Immediate(Len); break;
    // (Inc,Dec,Call,Call FAR,Jmp,Jmp FAR,Push)  Addr.
    case 0xff : Len = 2;
		switch(Reg)
		{
		  case 0 : str = "INC ";
			   if (Mod!=3 && RM==6) str += ByteorWordPtr[W];
			   str += Operand2(Mod,RM,1,2); break;
		  case 1 : str = "DEC ";
			   if (Mod!=3 && RM==6) str += ByteorWordPtr[W];
			   str += Operand2(Mod,RM,1,2); break;
		  case 2 : str = "CALL ";
			   str += Operand2(Mod,RM,1,2); break;
		  case 3 : str = "CALL FAR ";
			   str += Operand2(Mod,RM,1,2); break;
		  case 4 : str = "JMP ";
			   str += Operand2(Mod,RM,1,2); break;
		  case 5 : str = "JMP FAR ";
			   str += Operand2(Mod,RM,1,2); break;
		  case 6 : str = "PUSH ";
			   str += Operand2(Mod,RM,1,2); break;
		  case 7 : str = "??? ";
		}
		break;
    // Push Reg.
    case 0x50 : case 0x51 :
    case 0x52 : case 0x53 :
    case 0x54 : case 0x55 :
    case 0x56 : case 0x57 :
		Reg = Opcode & 7; Len = 1;
		str = "PUSH "; str += Operand1(Reg,1); break;
    // Push SReg.
    case 0x06 : case 0x0e :
    case 0x16 : case 0x1e :
		Reg = (Opcode >> 3) & 3; Len = 1;
		str = "PUSH "; str += Operand1(Reg,1,TRUE); break;
    // Pop Sreg.
    case 0x07 : case 0x0f :
    case 0x17 : case 0x1f :
		Reg = (Opcode >> 3) & 3; Len = 1;
		str = "POP "; str += Operand1(Reg,1,TRUE); break;
    // Pop [mem], Pop [reg].
    case 0x8f : str = "POP "; Len = 2; str += Operand2(Mod,RM,1,2); break;
    // Pop Reg.
    case 0x58 : case 0x59 :
    case 0x5a : case 0x5b :
    case 0x5c : case 0x5d :
    case 0x5e : case 0x5f :
		Reg = Opcode & 7; Len = 1;
		str = "POP "; str += Operand1(Reg,1); break;
    // ArithOper Addr,Imm ( Includes add,sub,adc,sbb,and,or,xor,cmp )
    case 0x80 : case 0x81 : case 0x82 : case 0x83 :
		ArithOper = Reg; Len = 2;
		str = ArithOperstr[ArithOper];
		if (RM==6 && Mod!=3) str += ByteorWordPtr[W];
		str += Operand2(Mod,RM,W,2) + ",";
		if (W) W = 1-D;
		str += Immediate(Len); break;
    // ArithOper AL,Imm.
    case 0x04 : case 0x0c : case 0x14 : case 0x1c :
    case 0x24 : case 0x2c : case 0x34 : case 0x3c :
		str = ArithOperstr[ArithOper]; Len = 2;
		str += "AL,"; str += ImmediateByte(1); break;
    // ArithOper AX,Imm.
    case 0x05 : case 0x0d : case 0x15 : case 0x1d :
    case 0x25 : case 0x2d : case 0x35 : case 0x3d :
		str = ArithOperstr[ArithOper]; Len = 3;
		str += "AX,"; str += ImmediateWord(1); break;
    // ArithOper Addr,Addr.
    case 0x00 : case 0x01 : case 0x02 : case 0x03 :
    case 0x08 : case 0x09 : case 0x0a : case 0x0b :
    case 0x10 : case 0x11 : case 0x12 : case 0x13 :
    case 0x18 : case 0x19 : case 0x1a : case 0x1b :
    case 0x20 : case 0x21 : case 0x22 : case 0x23 :
    case 0x28 : case 0x29 : case 0x2a : case 0x2b :
    case 0x30 : case 0x31 : case 0x32 : case 0x33 :
    case 0x38 : case 0x39 : case 0x3a : case 0x3b :
		str = ArithOperstr[ArithOper]; Len = 2;
		if (D==0)
		{
		  str += Operand2(Mod,RM,W,2) + "," + Operand1(Reg,W);
		}
		else
		{
		  str += Operand1(Reg,W) + "," + Operand2(Mod,RM,W,2);
		}
		break;
    // Inc Reg16.
    case 0x40 : case 0x41 : case 0x42 : case 0x43 :
    case 0x44 : case 0x45 : case 0x46 : case 0x47 :
    // Dec Reg16.
    case 0x48 : case 0x49 : case 0x4a : case 0x4b :
    case 0x4c : case 0x4d : case 0x4e : case 0x4f :
		str = (Opcode & 8) ? "DEC " : "INC ";
		Reg = Opcode & 7; Len = 1;
		str += Operand1(Reg,1); break;
    // Inc Reg8. & Dec Reg8.
    case 0xfe : Opcode = Ip[1];
		str = (Opcode & 8) ? "DEC " : "INC ";
		Reg = Opcode & 7; Len = 2;
		str += Operand1(Reg,0); break;
    // Test Addr,Addr.
    case 0x84 : case 0x85 :
		str = "TEST "; Len = 2;
		if (D==0) str += Operand1(Reg,W) + "," + Operand2(Mod,RM,W,2);
		else str += Operand2(Mod,RM,W,2) + "," + Operand1(Reg,W);
		break;
    // Test AL,Imm.
    case 0xa8 : str = "TEST AL,"; str += ImmediateByte(1); Len = 2; break;
    // Test AX,Imm.
    case 0xa9 : str = "TEST AX,"; str += ImmediateWord(1); Len = 3; break;
    // if (Reg==0) Test Addr,Immediate.
    // else SecArithOper Addr.
    case 0xf6 : case 0xf7 :
		ArithOper = Reg;
		Reg = RM; Len = 2;
		str = SecArithOperstr[ArithOper];
		if (Mod==6) str += ByteorWordPtr[W];
		if (ArithOper == 0)		// Test.
		{
		  str += Operand2(Mod,RM,W);
		  str += ",";
		  str += Immediate(2);
		}
		else str+=Operand2(Mod,RM,W,2);
		break;
    // (SHL,SHR,SAL,SAR) mem/reg,1.
    case 0xd0 : case 0xd1 : case 0xd2 : case 0xd3 :
		str = LogOperstr[Reg];
		if (RM==6)
		{
		  str += ByteorWordPtr[W]; Len = 4;
		  str += "["; str += ImmediateWord(2); str += "]";
		}
		else
		{
		  str += Operand1(RM,W); Len = 2;
		}
		str += (D)?",CL":",1";
		break;
    // Call Imm.
    case 0xe8 : str = "CALL "; Len = 3;
		str += (Word)(regIP + Len + (*(int FAR *)(Ip + 1)));
		break;
    // Call FAR Imm.
    case 0x9a : str = "CALL "; Len = 5;
		str += ImmediateWord(3); str += ":";
		str += ImmediateWord(1); break;
    // Jmp Imm.(2 byte displacement).
    case 0xe9 : str = "JMP "; Len = 3;
		str += (Word)(regIP + Len + (*(int FAR *)(Ip + 1)));
		break;
    // Jmp FAR Imm.
    case 0xea : str = "JMP "; Len = 5;
		str += ImmediateWord(1); str += ":";
		str += ImmediateWord(3); break;
    // Jmp short Imm.(-128 to +127 displacement).
    case 0xeb : str = "JMP "; Len = 2;
		str += (Word)(regIP + Len + (int)Ip[1]);
		break;
    // Conditional jmps.
    case 0x70 : str = "JO ";  Len = 2; AddOffset(str); break;
    case 0x71 : str = "JNO "; Len = 2; AddOffset(str); break;
    case 0x72 : str = "JB ";  Len = 2; AddOffset(str); break;
    case 0x73 : str = "JNB "; Len = 2; AddOffset(str); break;
    case 0x74 : str = "JZ ";  Len = 2; AddOffset(str); break;
    case 0x75 : str = "JNZ "; Len = 2; AddOffset(str); break;
    case 0x76 : str = "JBE "; Len = 2; AddOffset(str); break;
    case 0x77 : str = "JA ";  Len = 2; AddOffset(str); break;
    case 0x78 : str = "JS ";  Len = 2; AddOffset(str); break;
    case 0x79 : str = "JNS "; Len = 2; AddOffset(str); break;
    case 0x7a : str = "JPE "; Len = 2; AddOffset(str); break;
    case 0x7b : str = "JPO "; Len = 2; AddOffset(str); break;
    case 0x7c : str = "JL ";  Len = 2; AddOffset(str); break;
    case 0x7d : str = "JGE "; Len = 2; AddOffset(str); break;
    case 0x7e : str = "JLE "; Len = 2; AddOffset(str); break;
    case 0x7f : str = "JG ";  Len = 2; AddOffset(str); break;

    // Loops.
    case 0xe0 : str = "LOOPNZ "; Len = 2; AddOffset(str); break;
    case 0xe1 : str = "LOOPZ "; Len = 2; AddOffset(str); break;
    case 0xe2 : str = "LOOP "; Len = 2; AddOffset(str); break;
    case 0xe3 : str = "JCXZ "; Len = 2; AddOffset(str); break;

    case 0x26 : str = "ES:"; break;
    case 0x2e : str = "CS:"; break;
    case 0x36 : str = "SS:"; break;
    case 0x3e : str = "DS:"; break;

    case 0x27 : str = "DAA"; break;
    case 0x2f : str = "DAS"; break;
    case 0x37 : str = "AAA"; break;
    case 0x3f : str = "AAS"; break;

    case 0x90 : str = "NOP"; break;
    case 0x91 : case 0x92 : case 0x93 : case 0x94 :
    case 0x95 : case 0x96 : case 0x97 :
		str = "XCHG ";
		Reg = Opcode & 7;
		str += Operand1(Reg,1) + ",AX";
		break;
    case 0x98 : str = "CBW"; break;
    case 0x99 : str = "CWD"; break;
    case 0x9b : str = "WAIT"; break;
    case 0x9c : str = "PUSHF"; break;
    case 0x9d : str = "POPF"; break;
    case 0x9e : str = "SAHF"; break;
    case 0x9f : str = "LAHF"; break;

    case 0xa4 : str = "MOVSB"; break;
    case 0xa5 : str = "MOVSW"; break;
    case 0xa6 : str = "CMPSB"; break;
    case 0xa7 : str = "CMPSW"; break;
    case 0xaa : str = "STOSB"; break;
    case 0xab : str = "STOSW"; break;
    case 0xac : str = "LODSB"; break;
    case 0xad : str = "LODSW"; break;
    case 0xae : str = "SCASB"; break;
    case 0xaf : str = "SCASW"; break;

    case 0xc2 : str = "RET "; str += ImmediateWord(1); break;
    case 0xc3 : str = "RET"; break;
    case 0xca : str = "RETF "; str += ImmediateWord(1); break;
    case 0xcb : str = "RETF"; break;
    case 0xcd : str = "INT "; str += ImmediateByte(1); Len = 2; break;
    case 0xce : str = "INTO"; break;
    case 0xcf : str = "IRET"; break;

    case 0xd5 : str = "AAD"; break;
    case 0xd7 : str = "XLAT"; break;
    case 0xda : str = "AAM"; break;

    case 0xf0 : str = "LOCK"; break;
    case 0xf2 : str = "REPNZ"; break;
    case 0xf3 : str = "REPZ"; break;
    case 0xf4 : str = "HLT"; break;
    case 0xf5 : str = "CMC"; break;
    case 0xf8 : str = "CLC"; break;
    case 0xf9 : str = "STC"; break;
    case 0xfa : str = "CLI"; break;
    case 0xfb : str = "STI"; break;
    case 0xfc : str = "CLD"; break;
    case 0xfd : str = "STD"; break;

    // Xchg Addr,Addr.
    case 0x86 :
    case 0x87 : str = "XCHG "; Len = 2; str += GetOperands();
		break;
    // In.
    case 0xec : str = "IN AL,DX"; break;
    case 0xed : str = "IN AX,DX"; break;
    case 0xe4 : case 0xe5 :
		str = "IN "; str += Operand1(Reg,W); str += ",";
		str += ImmediateByte(1); Len = 3;
		break;
    // Out.
    case 0xee : str = "OUT DX,AL"; break;
    case 0xef : str = "OUT DX,AX"; break;
    case 0xe6 : case 0xe7 :
		str = "OUT "; str += Operand1(Reg,W); str += ",";
		str += ImmediateByte(1); Len = 3;
		break;
    // Lea Reg, mem.
    case 0x8d : str = "LEA "; Len = 2; str += Operand1(Reg,W); str += ",";
		str += Operand2(Mod,RM,W,2);
		break;
    // les Reg, mem.
    case 0xc4 : W = 1; D = 1-D; Len = 2;
		str = "LES "; str += GetOperands();
		break;
    // lds Reg, mem.
    case 0xc5 : W = 1; D = 1-D; Len = 2;
		str = "LDS "; str += GetOperands();
		break;
    // Unknown.
    default   : str = "????";
  }
  Length = Len;
  regIP += Len;
  return str;
}
Word Disasm::DispPage(int NumLines)
{
  Word PageSize=0;
  Byte FAR *Insts = CodePtr + regIP;
  while(NumLines--)
  {
    int InstLength;
    String Instr = GetAsm(Insts,InstLength);
    printf("%Fp ",Insts);
    for(int i=0;i<InstLength;i++)
      printf("%2.2X",Insts[i]);
    if (InstLength<3) printf("\t");
    printf("\t%s\n",(char *)Instr);
    PageSize += InstLength;
    Insts += InstLength;
  }
  return PageSize;
}
Dword Disasm::GetProcSize(Byte FAR *ProcBegin)
{
  Byte FAR *ProcAddr = ProcBegin;
  int InstLen;
  do
  {
    TraceInst(ProcAddr,InstLen);
    ProcAddr += InstLen;
  }while(CurInst.Instr != RET);
  ProcAddr -= InstLen;
  return (Dword)(ProcAddr - ProcBegin);
}