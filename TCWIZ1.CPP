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
//	File : TcWiz1.Cpp
//	Implementation of the DecodeSwitchStatement function.
//	Implementation file of TcWizard.h
//
#include <dos.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <conio.h>
#include "Disc.h"
#include "TcWizard.h"
#include "Disasm.h"

/*----------------------------------------------------------------------
FillSwitchInfo - Only to fill the info into the structure passed.
----------------------------------------------------------------------*/
void TcWizard::FillSwitchInfo(Byte FAR *Inst,InstInfo &inst,SwitchInfo &SwInfo)
{
    int SwitchType = IsSwitchStatement(FP_OFF(Inst));
    InstInfo tinst;
    if (SwitchType==C_SWITCH)
    {
        // An ordinary switch is coded like given below.
        // MOV CX,NumClauses
        // MOV BX,SwitchTable
        // CS:
        // MOV AX,[BX]
        // CMP AX,...
        // ...
        // LOOP label
        // JMP AddressOfDefault.
        // so, to find the address of the switch table, use BX
        //         to find the address of the no. of switch clauses, use CX.
        SwInfo.NumClauses = Regs[REG_CX].ValueFromHex();
        Word Segment = FP_SEG(Inst);	// CS
        SwInfo.SwitchTable= (Byte FAR *)MK_FP(Segment,Regs[REG_BX].ValueFromHex());

        FindInstruction(Inst,JMP,&tinst);
        SwInfo.DefaultAddress = tinst.Data11;
        SwInfo.DeleteAfterUse=0;
    }
    else if (SwitchType==C_SWITCH_ORDERED)
    {
        // An ordered switch is coded like given below.
        // MOV Reg,val
        // CMP Reg,MAXVAL        where MAXVAL gives the count of the 'case' statements.
        // JA    AddressOfDefault
        // SHL Reg,1
        // CS:
        // JMP [Reg+AddressOfSwitchTable]
        // Now we are in the "CMP Reg,MAXVAL" instruction.
        SwInfo.NumClauses = inst.Data21+1;
        Disasm d; d.SetCodePtr(CodePtr);
        int InstLen;
        d.TraceInst(Inst,InstLen); Inst+= InstLen;	// Skip the CMP instruction.
        d.TraceInst(Inst,InstLen); Inst+= InstLen;    // Trace the JA instruction.
        SwInfo.DefaultAddress = d.CurInst.Data11;
        d.TraceInst(Inst,InstLen); Inst+= InstLen;	// Ignore the next SHL & CS:
        d.TraceInst(Inst,InstLen); Inst+= InstLen;    //    instructions.
        Word Segment = FP_SEG(Inst);	// CS

        d.TraceInst(Inst,InstLen);
        Word FAR *Addrs = (Word FAR *)MK_FP(Segment,d.CurInst.Data12);
        int *tab = new int[(SwInfo.NumClauses+1)*2];
        assert(tab!=NULL);
        for(int i=0;i<SwInfo.NumClauses;i++)
            tab[i] = i;
        for(int j=0;i<2*SwInfo.NumClauses;i++,j++)
            tab[i] = Addrs[j];
        SwInfo.SwitchTable= (Byte FAR *)tab;
        // Memory allocated here is to be freed by 'DecodeSwitchStatement' fn.
        SwInfo.DeleteAfterUse=1;
    }
}

/*----------------------------------------------------------------------
FindInstruction - finds the next matching instructin and stores it
in the InstInfo passed.
Also returns the address of the instruction.
----------------------------------------------------------------------*/
Byte FAR *TcWizard::FindInstruction(Byte FAR *Inst,Byte Instruc,InstInfo *inst,int *LenPtr)
{
    Disasm d;
    d.SetCodePtr(CodePtr);
    int InstLen=0;
    do
    {
        Inst += InstLen;
        d.TraceInst(Inst,InstLen);
    }while(d.CurInst.Instr != Instruc);
    *inst = d.CurInst;
    if (LenPtr) *LenPtr = InstLen;
    return Inst;
}
/*----------------------------------------------------------------------
DecodeSwitchStatement - Decodes the switch statement and prints it into
the string list passed.
----------------------------------------------------------------------*/
Byte FAR *TcWizard::DecodeSwitchStatement(SwitchInfo &SwInfo,int How,StringList *sList,Word BlockEndAddress)
{
    int i;
    String s_LineBuffer;
    char *LineBuffer = (char *)s_LineBuffer;
    InstInfo inst;
    Dword BlockSize;
    Byte FAR *Inst;
    // How to find the end of the switch statement? Here we check for the
    //    addresses given in the JMPs at the end of each CASE block, and find
    //    the address which is nearest and after the address of DEFAULT stat.
    //    (Confusing? Think for some time. 'tis a weird logic, but works here.)
    Word EndOfSwitch = 0xffff;
    Word *SwitchAddrs = (Word *)new Word[SwInfo.NumClauses+1];
    assert(SwitchAddrs);
    for(i=0;i<SwInfo.NumClauses;i++)
    {
        // 2 - denotes the sizeof(int). Only integers can be used in SWITCH.
        Word addr = *(Word FAR *)(SwInfo.SwitchTable+i*2+SwInfo.NumClauses*2);
        for(int j=0;j<i;j++)
            if (addr<SwitchAddrs[j])
                break;
        for(int k=SwInfo.NumClauses;k>j;k--)
            SwitchAddrs[k] = SwitchAddrs[k-1];
        SwitchAddrs[j] = addr;

        Inst = (Byte FAR *)MK_FP(FP_SEG(CodePtr),addr);
        BlockSize = FindInstruction(Inst,JMP,&inst) - Inst;
        if (inst.Data11>=SwInfo.DefaultAddress && inst.Data11<EndOfSwitch)
            EndOfSwitch = inst.Data11;
    }
    if (EndOfSwitch>BlockEndAddress) EndOfSwitch=BlockEndAddress;

    SwitchAddrs[SwInfo.NumClauses] = SwInfo.DefaultAddress;
    // Now we have the EndOfSwitch address in out hands.
    Disasm d;
    d.SetCodePtr(CodePtr);
    for(i=0;i<SwInfo.NumClauses;i++)
    {
        Word addr = SwitchAddrs[i];
        Word FAR *AddrTable = ((Word FAR *)SwInfo.SwitchTable)+SwInfo.NumClauses;
        for(int j=0;j<SwInfo.NumClauses;j++)
            if (AddrTable[j]==addr)
                break;
        // 2 - denotes the sizeof(int). Only integers can be used in SWITCH.
        sprintf(LineBuffer,"case %d :",(int)*((signed short int FAR *)(SwInfo.SwitchTable+j*2)));
        InstAddr=addr;
        Output(LineBuffer,How,sList);
        Inst = (Byte FAR *)MK_FP(FP_SEG(CodePtr),addr);
        BlockSize = SwitchAddrs[i+1]-SwitchAddrs[i];
        int InstLen;
        d.TraceInst(Inst+BlockSize-2,InstLen);	//2=sizeof(JMP short_address)
        int BreakPresent=0;
        // If a BREAK is present in the block, we shall give it here, but if
        //    it is terminated by a GOTO to another place, include it in the
        //    block to be translated.
        if (d.CurInst.Instr==JMP && d.CurInst.Data11==EndOfSwitch)
        {
            BreakPresent=1;
            BlockSize -= InstLen;
        }
        int tStackTop = StackTop;
        TranslateBlock(Inst,BlockSize,How,sList);
        StackTop = tStackTop; Stack[StackTop]="";
        if (BreakPresent) Output("break;",How,sList);
        // How to find the end of the switch statement? Here we check for the
        //    addresses given in the JMPs at the end of each CASE block, and find
        //    the address which is nearest and after the address of DEFAULT stat.
        //    (Confusing? Think for some time. 'tis a weird logic, but works here.)
        if (inst.Data11>SwInfo.DefaultAddress && inst.Data11<EndOfSwitch)
            EndOfSwitch = inst.Data11;
    }
    InstAddr=SwInfo.DefaultAddress;
    Output("default :",How,sList);
    Inst = (Byte FAR *)MK_FP(FP_SEG(CodePtr),SwInfo.DefaultAddress);
    BlockSize = EndOfSwitch - SwInfo.DefaultAddress;
    TranslateBlock(Inst,BlockSize,How,sList);
    // inst contains the final JMP to outside the switch statement.
    Inst = (Byte FAR *)MK_FP(FP_SEG(CodePtr),EndOfSwitch);
    delete[] SwitchAddrs;

    if (SwInfo.DeleteAfterUse)
    {
        delete[] ((void *)SwInfo.SwitchTable);
        SwInfo.SwitchTable=NULL;
    }
    return Inst;
}
/*----------------------------------------------------------------------
GetReg - called to get the string containing the register's contents.
    This function checks if the string is empty and gives an error if so.
-----------------------------------------------------------------------*/
String &TcWizard::GetReg(int whichReg)
{
    if (Regs[whichReg].IsEmpty())
    {
        sprintf(errorStr,"Contents of empty register being accessed at %X",InstAddr);
        ErrorList.Add(errorStr);
    }
    return Regs[whichReg];
}
/*
[-----------------------------------------------------------------------]
[ GetLabel - returns 1 if the Address passed corresponds to a label	]
[    and also stores the label name in the string passed.			]
[-----------------------------------------------------------------------]
*/
int TcWizard::GetLabel(Word Address,String &s)
{
    register int i,labelcount;
    ((char *)s)[0]=0;
//    bsearch(&Address,LabelTable,NumLabels,sizeof(Word),sortFunc);
    for(i=0,labelcount=1;i<NumBlocks;i++)
    {
        if (BlockTable[i].Type==C_GOTO)
        {
            labelcount++;
            if (BlockTable[i].Address==Address)
            {
	            s = "Label";
	            s += labelcount;
	            return 1;
            }
        }
    }
    return 0;
}
/*----------------------------------------------------------------------
	Procedures for supporting the long-int datatype are below.
----------------------------------------------------------------------*/
/*----------------------------------------------------------------------
IsLongInt - returns 1 if the address given is part of any known long-int.
Uses the GlobalVars & LocalVars variable-lists.
----------------------------------------------------------------------*/
int TcWizard::IsLongInt(Variable &v1)
{
    Variable v2; int v2Type;
    // Is it part of any known variable?
    if (LocalVars.PartOfAnyVariable(v1,v2,v2Type) ||
            GlobalVars.PartOfAnyVariable(v1,v2,v2Type))
                return (v2Type==VAR_LONG);
    return 0;
}
/*----------------------------------------------------------------------
FirstWordOfLongInt - returns 1 if the address given is the 1st word of
 any known long-int.    Uses the GlobalVars & LocalVars variable-lists.
----------------------------------------------------------------------*/
int TcWizard::IsFirstWordOfLongInt(Variable &v1)
{
    Variable v2; int v2Type;
    // Is it part of any known variable?
    if (LocalVars.PartOfAnyVariable(v1,v2,v2Type) ||
            GlobalVars.PartOfAnyVariable(v1,v2,v2Type))
    {
        // Is it a long-int?
        if (v2Type==VAR_LONG)
            // Is it the 1st word of a long-int?
            return (v1.GetAddress()==v2.GetAddress());
    }
    return 0;
}
/*----------------------------------------------------------------------
IsInternalLongIntProc - returns 1 if the address given points to one
 of the procedures for long-int manipulation.
eg: LXMUL@ - multiplies two long-ints passed in DX:AX & BX:CX
 *** Makes use of the LongIntProcTable, hence the procedure must have
 *** been previously searched in the library and registered by calling
 *** RegisterLongIntProc function.
----------------------------------------------------------------------*/
int TcWizard::IsInternalLongIntProc(InstInfo &inst)
{
    Variable v;
    inst.GetVariable(v,1);
    char *ProcName = GlobalVars.GetName(v);
    if (!ProcName) return 0;
    char *NameEnd = ProcName + strlen(ProcName);
    if (strncmp(NameEnd-4,"MUL@",4)==0) LongIntMultiply();
    else if (strncmp(NameEnd-4,"DIV@",4)==0) LongIntDivide("/");
    else if (strncmp(NameEnd-4,"MOD@",4)==0) LongIntDivide("%");
    else if (strncmp(NameEnd-4,"LSH@",4)==0) LongIntShift("<<");
    else if (strncmp(NameEnd-4,"RSH@",4)==0) LongIntShift(">>");
    else return 0;
    return 1;
}
#define IsHexDigit(x)    ((x>='0' && x<='9') || \
			(x>='A' && x<='F') || \
			(x>='a' && x<='f'))
int IsOnlyNumber(String &s)
{
    char *ptr = (char *)s;
    if (!*ptr) return 0;
    while(*ptr)
    {
        if(!IsHexDigit(*ptr)) return 0;
        ptr++;
    }
    return 1;
}
void TcWizard::LongIntShift(char *Oper)
{
    String oper1,oper2;
    if (IsOnlyNumber(Regs[REG_DX]))
    {
        if(!IsOnlyNumber(Regs[REG_AX]))
        {
            sprintf(errorStr,"Error decoding long-int shifting code at %X",InstAddr);
            ErrorList.Add(errorStr);
        }
        else
            oper1=Regs[REG_DX]+Regs[REG_AX];
    }
    else oper1=Regs[REG_AX];
    oper2=Regs[REG_CL];
    String s=oper1+" "+Oper+" "+oper2;
    Regs[REG_AX]=s;
    oper1="HIWORD(";
    Regs[REG_DX]=oper1 + s + ")";
}
void TcWizard::LongIntDivide(char *Oper)
{
    String oper1,oper2;
    if (IsOnlyNumber(Stack[StackTop-1]))
    {
        if(!IsOnlyNumber(Stack[StackTop-2]))
        {
            sprintf(errorStr,"Error decoding long-int operation code at %X",InstAddr);
            ErrorList.Add(errorStr);
        }
        else
            oper1=Stack[StackTop-2]+Stack[StackTop-1];
    }
    else oper1=Stack[StackTop-1];
    if (IsOnlyNumber(Stack[StackTop-3]))
    {
        if(!IsOnlyNumber(Stack[StackTop-4]))
        {
            sprintf(errorStr,"Error decoding long-int division code at %X",InstAddr);
            ErrorList.Add(errorStr);
        }
        else
            oper2=Stack[StackTop-4]+Stack[StackTop-3];
    }
    else oper2=Stack[StackTop-3];
    String s; s="("; s += oper1 +") " + Oper + " ("+ oper2+")";
    Regs[REG_AX]=s;
    oper1="HIWORD(";
    Regs[REG_DX]=oper1 + s + ")";
}
void TcWizard::LongIntMultiply()
{
    String oper1,oper2;
    if (IsOnlyNumber(Regs[REG_CX]))
    {
        if(!IsOnlyNumber(Regs[REG_BX]))
        {
            sprintf(errorStr,"Error decoding long-int multiply code at %X",InstAddr);
            ErrorList.Add(errorStr);
        }
        else
            oper1=Regs[REG_CX]+Regs[REG_BX];
    }
    else oper1=Regs[REG_BX];
    if (IsOnlyNumber(Regs[REG_DX]))
    {
        if(!IsOnlyNumber(Regs[REG_AX]))
        {
            sprintf(errorStr,"Error decoding long-int multiply code at %X",InstAddr);
            ErrorList.Add(errorStr);
        }
        else
            oper2=Regs[REG_DX]+Regs[REG_AX];
    }
    else oper2=Regs[REG_AX];
    String s; s="("; s += oper1 +") * ("+ oper2+")";
    Regs[REG_AX]=s;
    oper1="HIWORD(";
    Regs[REG_DX]=oper1 + s + ")";
}
void TcWizard::FindAndRegisterDataTypes(InstInfo &inst)
{
//    return;
    Variable v;
    int size=-1;
    switch(inst.Operand1)
    {
        case REG_DIRECT :
	switch(inst.Operand2)
	{
	    case MEMORY :
	    case INDEXED_BYTE :
	    case INDEXED_WORD :
		inst.GetVariable(v,2);
		// Is it operating on a 8-bit register?
		size=(inst.Data11 < REG_AX)?0:1;
	}
	break;
        case MEMORY :
        case INDEXED_BYTE :
        case INDEXED_WORD :
	inst.GetVariable(v,1);
	switch(inst.Operand2)
	{
	    case IMMEDIATE : size=inst.operSize1; break;
	    case REG_DIRECT: size=(inst.Data21 < REG_AX)?0:1; break;
	}
    }
    // Add new datatype only if it is not registered previously as a long int.
    if (!IsLongInt(v))
    {
        if (size==0)
            AddVar(v,VAR_CHAR,1);
        else if (size==1)
            AddVar(v,VAR_INT,2);
    }
}
void TcWizard::AddVar(Variable &v,int type,int size)
{
    VariableList *vList=NULL;
    switch(v.Operand)
    {
        case IMMEDIATE :
	        if (type==VAR_LIB_FUNCTION || type==VAR_USER_FUNCTION)
	            vList=&GlobalVars;
	        break;
        case INDEXED_BYTE :
        case INDEXED_WORD :
	        vList=&LocalVars;
            break;
        case MEMORY :
            vList=&GlobalVars;
            break;
    }
    if (vList)
        vList->Add(v,type,size);
}
void TcWizard::DelVar(Variable &v)
{
    VariableList *vList=NULL;
    switch(v.Operand)
    {
        case IMMEDIATE :
	        vList=&GlobalVars;
            break;
        case INDEXED_BYTE :
        case INDEXED_WORD :
	        vList=&LocalVars;
            break;
        case MEMORY :
            vList=&GlobalVars;
            break;
    }
    if (vList)
        vList->Del(v);
}
void TcWizard::DeCompileFull()
{
    Byte FAR *ProcAddr;
    Pass1(GetMain(),1);
    FlushAll();
    GlobalVars.NameUnknownVariables();
    VariableList *vList=&GlobalVars;
    do
    {
        if (vList->Type==VAR_USER_FUNCTION)
        {
            ProcAddr = (Byte FAR *)MK_FP(vList->Data.Data2,vList->Data.Data1);
            Pass1(ProcAddr);
/*            ctr=SkipProcInit(ProcAddr,ProcSize);
            // Register the local variables used.
            TranslateBlock(ProcAddr+ctr,ProcSize,TcWizard::SEPARATE_LINES,NULL);
            // Now do the actual display*/
            LocalVars.NameUnknownVariables();
            LocalVars.MakeAllReadOnly();
            DisplayProc(ProcAddr);
            LocalVars.Flush();
        }
        vList=vList->Next;
    }while(vList);
}
/*--------------------------------------------------------------
IsReturnStatement - finds if the current instruction ('JMP') is
 actually a jmp to the procedure's end.    if so,    this is a
 return statement.
--------------------------------------------------------------*/
int TcWizard::IsReturnStatement()
{
/*    Byte FAR *Inst = (Byte FAR *)MK_FP(FP_SEG(CodePtr),InstAddr);
    Disasm d;
    do
    {
        // have we reached the end of the procedure?
        if (Inst>=ProcEndAddress) return 1;
        // if no, check this instruction.
        int InstLen;
        d.TraceInst(Inst,InstLen);
        if (d.CurInst.Instr==JMP)
            Inst=(Byte FAR *)MK_FP(d.CurInst.Data12,d.CurInst.Data11);
    }while(d.CurInst.Instr==JMP);*/
    return 0;
}
