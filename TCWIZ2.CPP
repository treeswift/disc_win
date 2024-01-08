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
//	File : TcWiz2.Cpp
//	Implementation of the Translate Block function.
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
Wrapper Fn. to find Modified Registers. Only used by IsTernaryOperatorCode.
----------------------------------------------------------------------*/
void TcWizard::FindRegsModified(Byte FAR *BlockBegin,Sword BlockSize,Byte *RegModified)
{
    TranslateBlock(BlockBegin,BlockSize,0,NULL,RegModified,NULL);
}
/*----------------------------------------------------------------------
Wrapper Fn. to find 'Used' registers. Only used by IsTernaryOperatorCode.
----------------------------------------------------------------------*/
void TcWizard::FindRegsUsed(Byte FAR *BlockBegin,Sword BlockSize,Byte *RegModified,Byte *RegUsed)
{
    TranslateBlock(BlockBegin,BlockSize,0,NULL,RegModified,RegUsed);
}
/*----------------------------------------------------------------------
Overloaded Fn. which ignores RegFlags parameter, i.e. not checking which
registers are modified in the code block.
----------------------------------------------------------------------*/
void TcWizard::TranslateBlock(Byte FAR *BlockBegin,Sword BlockSize,int How,StringList *sList)
{
    TranslateBlock(BlockBegin,BlockSize,How,sList,NULL,NULL);
}
/*
[-----------------------------------------------------------------------]
[ TranslateBlock							]
[-----------------------------------------------------------------------]
[ Converts the given block of code into C-code.				]
[									]
[ BlockBegin - Starting of the code block				]
[ BlockEnd   - End of the code fragment					]
[ How	     - How to store in the StringList				]
[		- as SEPARATE_LINES or COMMA_EXPRESSIONS		]
[ sList      - StringList in which the output lines are stored		]
[ RegModified- Array of flags to indicate which registers were		]
[		modified in the given code fragment			]
[ RegUsed    - Array of flags to indicate which registers are 'used'	]
[		i.e. accessed.						]
[		(Elements of both RegModified & RegUsed are set to 1 by	]
[		 this procedure, and this proc.does not make use of	]
[		 these parameters for anything else.)			]
[-----------------------------------------------------------------------]
*/
void TcWizard::TranslateBlock(
	Byte FAR *BlockBegin,
	Sword BlockSize,
	int How,
	StringList *sList,
	Byte *RegModified,
	Byte *RegUsed
	)
{
#define marknotused(Reg) NotUsed[Reg]=1;
#define markused(Reg) NotUsed[Reg]=0;
#define CheckAndOutput(str,how,slist) \
	if (!RegModified && !RegUsed) \
    { \
      Output(str,how,slist); \
    }
#define CheckAndDeletePending(Reg) \
	if (!RegModified && !RegUsed) \
    { \
      DeletePending(Reg); \
    }
#define CheckAndAddPending(str) \
	if (!RegModified && !RegUsed) \
    { \
      AddPending(str); \
    }
#define SetRegModified(Reg) \
	if (RegUsed && !RegUsed[Reg]) \
      RegModified[Reg]=1; \
	marknotused(Reg)
#define SetRegModified2(Reg) \
	if (RegUsed && !RegUsed[Reg]) \
      RegModified[Reg]=2; \
	marknotused(Reg)
#define SetRegUsed(Reg)     \
	if (RegUsed) \
      RegUsed[Reg]=1; \
	markused(Reg)
#define CheckAndSetRegUsed(Oper,Data) \
	if(Oper==REG_DIRECT && !RegisterVariable(Data)) \
    { \
      SetRegUsed(Data); \
    }
#define toString(str) toStr.Copy(str)

    Byte NotUsed[8+8+4];
    memset(NotUsed,0,sizeof(NotUsed));

    if (BlockSize<=0)
        return;

    int InstLen;
    int i;
    int row=wherey()+1;
    SwitchInfo SwInfo;	// Structure used for SWITCH statements.
    SwInfo.NumClauses=0;
    String s_LineBuffer;
    char *LineBuffer = (char *)s_LineBuffer;
    Byte FAR *Inst;
    Disasm d;
    d.SetCodePtr(CodePtr);
    Inst = BlockBegin;
    Dword ctr = 0;
    String tmp,tmp1,toStr;
    InstInfo inst = {}, previnst = {};
    char PrevImmValueStored=0;
    Word PrevImmediateValue;		// Used in converting 'long-int' code.
    Variable v1;
    char *vName;
    Word Segment;
    Byte FAR *p;
    int Use2ndData;
    int HiWordPrefix;
    int dec;
    Byte FAR *ProcBegin;
    Byte FAR *ProcEnd;

    while(ctr<(Dword)BlockSize && Inst<ProcEndAddress)
    {
        // Set the data segment for the next instruction as the usual data segment
        // if the previous instruction was not a segment override.
        Byte FAR *DataSegment;
        if (inst.Instr!=SEGMENT)
            DataSegment = GetDataSegment();
        DataSegment = DataSegment;

        InstAddr = FP_OFF(Inst);
        gotoxy(5,row); printf("Decoding Instruction at address % 4X\r",InstAddr);
        d.TraceInst(Inst,InstLen);
        inst = d.CurInst;
        TranslateTrickyInsts(inst);
        LineBuffer[0] = 0;
        // Are we moving the return value of any procedure (even after some
        //    arithmetic operation) to somewhere? If so,    delete the corresponding
        //    pending o/p line. But Make sure that the 2nd operand is a register
        if (inst.Operand2 == REG_DIRECT)
            CheckAndDeletePending(Regs[inst.Data21]);
        // Is it a single operand instruction?
        if (inst.Operand1 == REG_DIRECT && inst.Operand2==OPER_UNKNOWN)
            CheckAndDeletePending(Regs[inst.Data11]);
        // Is the current address given a label in the code?
        // If so, print the label.
        if (GetLabel(InstAddr,tmp))
        {
            tmp += ":";
            CheckAndOutput(tmp,How,sList);
        }
        // If this is a SWITCH statement, make necessary arrangements.
        if (IsSwitchStatement(InstAddr))
            FillSwitchInfo(Inst,inst,SwInfo);

        // Find the datatype of the memory operand used in this instruction
        // (if any) and register it in the LocalVars/GlobalVars list.
        if (inst.Instr!=LEA)	// since argument to LEA can be any datatype.
            FindAndRegisterDataTypes(inst);

        tmp1 = "";
        // Now trace the instruction.
        switch(inst.Instr)
        {
            // Segment override.
            case SEGMENT:
                Segment = GetReg(inst.Data11).ValueFromHex();
		        DataSegment = (Byte FAR *)MK_FP(Segment,0);
		        break;
            case CBW    :
                Regs[REG_AX] = /*toString("(int)") + */GetReg(REG_AL);
		        SetRegUsed(REG_AL);
		        break;
            case CWD    :
                /*if (strncmp((char *)GetReg(REG_AX),"(int)",5)==0)
		        strcpy((char *)GetReg(REG_AX),((char *)GetReg(REG_AX))+5);*/
		        tmp = GetReg(REG_AX);
		        //Regs[REG_AL] = toString("LOBYTE((long)") + tmp + ")";
		        Regs[REG_AX] = tmp;
		        Regs[REG_DX] = toString("HIWORD((long)") + tmp + ")";
		        SetRegUsed(REG_AX);
		        break;
            case LEA    :
                tmp=GetOperand2(inst);
		        Regs[inst.Data11] =    toString("&") + tmp;
		        if (RegisterVariable(inst.Data11))
		        {
		            sprintf(LineBuffer,"%s = _%s;",Regstr[i],GetRegVarName(inst.Data11));
		            CheckAndOutput(LineBuffer,How,sList);
		        }
		        else SetRegModified(inst.Data11);
		        break;
            case IDIV : case DIV :
		        switch(inst.Operand1)
		        {
		            case REG_DIRECT :
		            case MEMORY :
		            case REG_INDIRECT :
		            case INDEXED_BYTE :
		            case INDEXED_WORD :
			            CheckAndSetRegUsed(inst.Operand1,inst.Data11);
			            SetRegModified(REG_AL);
			            SetRegModified(REG_AX);
			            SetRegModified(REG_DX);

			            tmp=GetOperand1(inst);
			            if (strncmp(GetReg(REG_AX),"(long)",6)==0)
			                strcpy((char *)GetReg(REG_AX),((char *)GetReg(REG_AX))+6);
			            if (inst.operSize1==0)
			            {
			                Regs[REG_AL] = toString("(") + GetReg(REG_AX) + \
			                    ") / (" + tmp + ")";
			            }
			            else
			            {
			                tmp1 = toString("(") + GetReg(REG_AX) + ") / (" + \
				                    tmp + ")";
			                tmp = toString("(") + GetReg(REG_AX) + ") % (" + \
				                    tmp + ")";
			                Regs[REG_AL] = tmp1;
			                Regs[REG_AX] = tmp1;
			                Regs[REG_DX] = tmp;
			                Regs[REG_DL] = tmp;
			            }
			            break;
		        }
		        break;
            case IMUL : case MUL :
		        switch(inst.Operand1)
		        {
		            case REG_DIRECT :
		            case MEMORY :
		            case REG_INDIRECT :
		            case INDEXED_BYTE :
		            case INDEXED_WORD :
			            CheckAndSetRegUsed(inst.Operand1,inst.Data11);
			            SetRegModified(REG_AL);
			            SetRegModified(REG_AX);
			            SetRegModified(REG_DX);

			            tmp=GetOperand1(inst);
			            if (inst.operSize1==0)
			            {
			                Regs[REG_AX] = toString("(") + GetReg(REG_AL) + \
			                        ") * (" + tmp + ")";
			            }
			            else
			            {
			                tmp = toString("(") + GetReg(REG_AX) + ") * (" + \
				                            tmp + ")";
			                Regs[REG_AL] = tmp;
			                Regs[REG_AX] = tmp;
			                Regs[REG_DX] = toString("HIWORD(") + tmp + ")";
			            }
			            break;
		        }
		        break;
            case NOT    : case NEG :
		        switch(inst.Operand1)
		        {
		            case MEMORY :
		            case REG_INDIRECT :
		            case INDEXED_BYTE :
		            case INDEXED_WORD :
			            tmp = GetOperand1(inst);
ComPlaceForNot:
			            sprintf(EndOf(LineBuffer),"%s = %c (%s);",(char *)tmp,\
				            (inst.Instr==NOT)?'~':'-',\
				            (char *)tmp);
			            CheckAndOutput(LineBuffer,How,sList);
			            break;
		            case REG_DIRECT :
			            if (RegisterVariable(inst.Data11))
			            {
			                tmp=GetRegVarName(inst.Data11);
			                goto ComPlaceForNot;
			            }
			            else
			            {
			                Regs[inst.Data11] = toString((inst.Instr==NOT) ? \
				            "~ (":"- (") + GetReg(inst.Data11) + ")";
			                SetRegModified(inst.Data11);
			                SetRegUsed(inst.Data11);
			            }
			            break;
		        }
		        break;
            case CMP    :
                inst.GetVariable(v1,1);
    		    if (IsLongInt(v1) && !IsFirstWordOfLongInt(v1))
                    break;
		        inst.GetVariable(v1,2);
		        if (IsLongInt(v1) && !IsFirstWordOfLongInt(v1))
                    break;

    		    if (inst.Operand1==REG_DIRECT)
	    	        CheckAndDeletePending(GetReg(inst.Data11));
		        CheckAndSetRegUsed(inst.Operand1,inst.Data11);
		        CheckAndSetRegUsed(inst.Operand2,inst.Data21);
    		    // if we are just checking the registers modified in this
	    	    // part of the code, don't decode the switch statement.
		        if (!RegModified && !RegUsed)
		        {
    		        if (InSwitchStatement(SwInfo))
	    	        {
		                if (inst.Operand1!=REG_DIRECT)
		                {
			                sprintf(errorStr,"Error in decoding a Switch construct at %X",InstAddr);
			                ErrorList.Add(errorStr);
			                break;
		                }
		                if (IsSwitchStatement(InstAddr)==C_SWITCH_ORDERED)
		                {
			                if (inst.Operand2==IMMEDIATE) tmp=GetOperand1(inst);
			                else tmp=GetOperand1(inst);
		                }
		                else
		                {
			                if ((inst.Operand2==INDEXED_BYTE ||
			                         inst.Operand2==INDEXED_WORD) &&
			                    inst.Data21==INDX_BP) tmp=GetOperand2(inst);
                			else
                                tmp=GetOperand1(inst);
		                }
		                sprintf(LineBuffer,"switch(%s)",(char *)tmp);
		                CheckAndOutput(LineBuffer,How,sList);
		                CheckAndOutput("{",How,sList);

		                // skip all code upto the end of the switch statement.
		                Byte FAR *tInst = DecodeSwitchStatement(SwInfo,How,sList,FP_OFF(BlockBegin)+BlockSize);
		                // make adjustments to the counter variables.
		                ctr += tInst-Inst;
		                Inst = tInst;
		                // Just to fool the incrementing code at the end of
		                //    the switch.
		                ctr -= InstLen;
		                Inst -= InstLen;

		                CheckAndOutput("}",How,sList);
		                break;
		            }
		        }
		        CompareVar.Operand1=GetOperand1(inst);
		        CompareVar.Operand2=GetOperand2(inst);
		        break;
            case JMP    :
                /*if(IsReturnStatement())
		        {
		            tmp = "return ";
		            if (NotUsed[REG_DX])
		            {
		                CheckAndDeletePending(Regs[REG_DX]);
		                // if DX = HIWORD(some expression), ignore it.
		                if (strncmp(Regs[REG_DX],"HIWORD",6)!=0)
			    tmp=Regs[REG_DX];
		            }
		            if (!NotUsed[REG_DX] && NotUsed[REG_AX])
		            {
		                CheckAndDeletePending(Regs[REG_AX]);
		                tmp+=Regs[REG_AX];
		            }
		            tmp += ";";
		            sprintf(LineBuffer,"%s",(char *)tmp);
		        }
		        else*/
		        {
		            GetLabel(inst.Data11,tmp);
		            sprintf(LineBuffer,"goto %s;",(char *)tmp);
		        }
		        CheckAndOutput(LineBuffer,How,sList);
		        break;
            //	JA, JAE, JB, JBE, JZ, JNZ, JG, JGE, JL, JLE
            //	Others normally do not occur in C Programs. Hence ignored.
            case JA : case JAE : case JB : case JBE :
		        i=1;
                goto comjmp;

            case JL : case JG : case JZ :
            case JLE: case JGE: case JNZ:
		        i=0;
comjmp:
		        // No CMP instruction encountered before?
		        if (((char *)CompareVar.Operand1)[0]==0)
		        {
		            //printf("\nUnknown Code Structure at %X\n",InstAddr);
		            break;
		        }
		        sprintf(LineBuffer,"if(");
		        //if (i) sprintf(EndOf(LineBuffer),"(unsigned)");
		        sprintf(EndOf(LineBuffer),"(%s)",\
			        (char *)CompareVar.Operand1);
		        sprintf(EndOf(LineBuffer)," %s ",\
			        CompOperators[inst.Instr - JA]);
		        //if (i) sprintf(EndOf(LineBuffer),"(unsigned)");
		        CompareVar.EndAddress = inst.Data11;
		        GetLabel(inst.Data11,tmp);
		        sprintf(EndOf(LineBuffer),"(%s))",(char *)CompareVar.Operand2);
		        p=Inst+InstLen;
		        // If we are just checking the registers modified in this
		        // part of the code, don't decode ternary operators etc.
		        if (!RegUsed && !RegModified)
		        // else, Check for ternary operator code here.
		            p = CheckForTernaryOperCode(Inst+InstLen,LineBuffer+2);
		        if(p!=Inst+InstLen)
		        {
		            ctr += p-Inst;
		            Inst = p;
		            // Just to fool the incrementing code at the end of the proc.
		            InstLen=0;
		        }
		        else
		        {
		            sprintf(EndOf(LineBuffer)," goto %s;",(char *)tmp);
		            CheckAndOutput(LineBuffer,How,sList);
		        }
		        ((char *)CompareVar.Operand1)[0]=0;	// reset.
		        break;
            case SHL : case SHR :
		        if (inst.Operand2==IMMEDIATE)
		            tmp1 += inst.Data21;
		        else if (inst.Operand2==REG_DIRECT)
		            tmp1 += GetReg(inst.Data21);
		        else assert(0);
            case INC : case DEC :
		        switch(inst.Operand1)
		        {
		        case REG_DIRECT :
			    // If the register used is not a register variable...
			        if (!RegisterVariable(inst.Data11))
			        {
			            char *t;
			            switch(inst.Instr)
			            {
			                case INC : t="+1"; break;
			                case DEC : t="-1"; break;
			                case SHL : t="<<"; break;
			                case SHR : t=">>"; break;
			            }
			            Regs[inst.Data11] = toString("(") + \
				        GetReg(inst.Data11) + t + tmp1 + ")";
			            SetRegModified(inst.Data11);
			            SetRegUsed(inst.Data11);
			            break;
			        }
			    // If it is a register variable, or other operands,...
		        default :
			        tmp=GetOperand1(inst);
			        char *t;
			        switch(inst.Instr)
			        {
			            case INC : t="++"; break;
			            case DEC : t="--"; break;
			            case SHL : t="<<="; break;
			            case SHR : t=">>="; break;
			        }
			        sprintf(LineBuffer,"%s%s%s;",(char *)tmp,\
				        t,(char *)tmp1);
			        CheckAndOutput(LineBuffer,How,sList);
			        break;
		        }
		        break;
            case ADC    : case SBB :
		        // only the high words of the long-int are involved in
		        // ADC & SBB. Hence subtract 2 from the variable to get
		        // the actual address of the long int.
		        int whichOper;
		        whichOper=(inst.Operand1!=REG_DIRECT)?1:2;
		        if (whichOper==1)
                    inst.GetVariable(v1,1);
		        else
                    inst.GetVariable(v1,2);

		        DelVar(v1-2);
                DelVar(v1);
		        AddVar(v1-2,VAR_LONG,4);
		        if (inst.Instr==ADC)
                    inst.Instr=ADD;
		        else
                    inst.Instr=SUB;
		        goto ComForMovingData;

            // We get an 'LES' instruction while using 'FAR' pointers.
            // eg.    LES BX,[1000] means we are loading es:bx with FAR-pointer
            // located at 1000.    Here, I just ignore the 'es' part of the
            // instruction, and load BX with [1000].
            case LES :
                inst.Instr=MOV;
            case MOV :
            case ADD :
            case SUB :
            case AND :
            case OR  :
            case XOR :
		    // Check if the destination operand is a part of a long-int.
		    //    Execute the instruction only if it is the HIWORD of the
		    //    long-int.
ComForMovingData:
		        Use2ndData=0;
		        HiWordPrefix=0;
		        whichOper=(inst.Operand1!=REG_DIRECT)?1:2;

		        if (whichOper==1)
                    inst.GetVariable(v1,1);
		        else
                    inst.GetVariable(v1,2);

		        if (IsLongInt(v1))
		        {
		            // If it is not the 1st word of the long-int, ignore
		            //    the instruction.
		            if (!IsFirstWordOfLongInt(v1))
		            {
		                if (inst.Operand2==IMMEDIATE)
		                {
			                PrevImmValueStored=1;
			                PrevImmediateValue=inst.Data21;
		                }
		                if (whichOper==2)
		                {
			                inst.SetVariable(v1-2,2);
			                HiWordPrefix=1;
		                }
		                else break;
		            }
		            if (inst.Operand2==IMMEDIATE)
		            {
		                // This instruction operates on the 1st word using
		                //    an immediate value. Grab the 2nd word of the
		                //    immediate value - which could be the previous or
		                //    next instruction.
		                // If it has not been obtained from previous
		                //    instruction it must be in the next instruction.
		                if (!PrevImmValueStored)
		                {
			                Inst+=InstLen;
			                d.TraceInst(Inst,InstLen);
			                if (d.CurInst.Instr==ADC)
                                d.CurInst.Instr=ADD;
			                if (d.CurInst.Instr==SBB)
                                d.CurInst.Instr=SUB;
			                if(d.CurInst.Instr!=inst.Instr ||
			                     d.CurInst.Operand2!=IMMEDIATE)
			                {
			                    sprintf(errorStr,"Error understanding long int code at %X",InstAddr);
			                    ErrorList.Add(errorStr);
			                    break;
			                }
			                PrevImmediateValue=d.CurInst.Data21;
		                }
		                inst.Data22=inst.Data21;
		                inst.Data21=PrevImmediateValue;
		                PrevImmValueStored=0;
		                Use2ndData=1;
		            }
		        }
		        switch(inst.Operand1)
		        {
		        case REG_INDIRECT :
		        case INDEXED_BYTE :
		        case INDEXED_WORD :
			        tmp=GetOperand1(inst);
			        goto PrintVariableAssignment;
		        case MEMORY :
			        tmp=GetOperand1(inst);
PrintVariableAssignment:
        			tmp1=tmp;
	        		sprintf(LineBuffer,"%s %s= ",(char *)tmp,\
	        			ArithOperators[inst.Instr - MOV]);
			        switch(inst.Operand2)
			        {
			            case IMMEDIATE :
				            s_LineBuffer += (Word)inst.Data21;
				            if (Use2ndData)		// See above.
				                s_LineBuffer += (Word)inst.Data22;
				            break;
			            case REG_DIRECT :
				            CheckAndSetRegUsed(inst.Operand2,inst.Data21);
				            tmp=GetOperand2(inst);
				            strcat(LineBuffer,(char *)tmp);
				            // Is it a long-int?
				            if (strncmp((char *)tmp,"HIWORD",6)==0)
				                Use2ndData=1;	// Used in the checking below.
				            // If it is like MOV addr,REG then do a false
				            //    assignment MOV REG,addr so that any
				            //    following instructions of the format
				            //    MOV addr,REG are translated properly.
				            if (!RegisterVariable(inst.Data21))
				            {
				                tmp=GetOperand1(inst);
				                Regs[inst.Data21] = tmp;
				            }
				            break;
			            default :
				            tmp = GetOperand2(inst);
				            strcat(LineBuffer,(char *)tmp);
				            break;
			        }
			        sprintf(EndOf(LineBuffer),";");
			        CheckAndOutput(LineBuffer,How,sList);
			        // check for long-ints and register them
			        if (Use2ndData)
			        {
			            inst.GetVariable(v1,1);
			            if (!IsLongInt(v1))
			            {
			                DelVar(v1-2); DelVar(v1);
			                AddVar(v1-2,VAR_LONG,4);
			            }
			        }
			        break;
		        case REG_DIRECT :
			        tmp=GetOperand2(inst);
			        tmp1=tmp;
			        if (HiWordPrefix)
                        tmp=toString("HIWORD(")+tmp+")";
			        SetRegModified(inst.Data11);
			        CheckAndSetRegUsed(inst.Operand2,inst.Data21);

			        // Is it ADD SP,Imm?
			        if ((inst.Instr == ADD) && \
			                (inst.Operand2 == IMMEDIATE) && \
			                (inst.Data11 == Register(1,4)))
			        {
			            int dec = (int)inst.Data21 / sizeof(Word);
			            for(i=0;i<dec;i++,StackTop--)
			                Stack[StackTop-1] = "";
			            assert(StackTop>=0);		// Stack Underflow?
			            break;
			        }
			        if (inst.Instr == MOV)
			            Regs[inst.Data11] = tmp;
			        else
			        {
			            tmp = toString("(") + tmp + ")";
			            Regs[inst.Data11] = GetReg(inst.Data11) +    \
			                " " + ArithOperators[inst.Instr - MOV] \
			                + " " + tmp;
			        }
			        // Check if it is a register variable
			        // If so,    print it out the same way a memory
			        //    variable is assigned.
			        if (RegisterVariable(inst.Data11))
			        {
			            tmp=GetRegVarName(inst.Data11);
			            goto PrintVariableAssignment;
			        }
			        SetRegModified(inst.Data11);
			        break;
		        }
		        break;
            case PUSH :
                i = StackTop;
                assert(StackTop<NUM_STACK_LINES);
    		    CheckAndSetRegUsed(inst.Operand1,inst.Data11);
	    	    tmp=GetOperand1(inst);
		        Stack[i] += tmp;
		        UseAsFnParms[i]=1;

		        // Check for PUSHs involving part of long-ints.
		        inst.GetVariable(v1,1);
		        if (IsLongInt(v1))
		            UseAsFnParms[i]=IsFirstWordOfLongInt(v1);

		        // If it is a comma-expression, close the braces.
		        if (!IsEqualClosingParentheses(Stack[i]))
                    Stack[i] += ")";
		        // Clear the current stack string to blank.
		        StackTop++;
		        Stack[StackTop] = "";
		        TracingFunctionCall=1;
		        break;
            case POP :
                if (StackTop<=0)
		        {
		            sprintf(errorStr,"Procedure Stack Underflow at %X",InstAddr);
		            ErrorList.Add(errorStr);
		            break;
		        }
		        StackTop--;
		        switch(inst.Operand1)
		        {
		            case MEMORY :
		            case INDEXED_BYTE :
		            case INDEXED_WORD :
		                tmp = GetOperand1(inst);
		                sprintf(EndOf(LineBuffer),"%s = %s;",(char *)tmp,\
			                (char *)Stack[StackTop]);
		                CheckAndOutput(LineBuffer,How,sList);
		                break;
		            case REG_DIRECT :
		                Regs[inst.Data11] = Stack[StackTop];
		                SetRegModified(inst.Data11);
		                break;
		        }
		        Stack[StackTop] = "";
		        TracingFunctionCall=0;
		        break;
            case CALL :
                TracingFunctionCall=0;
    		    if (IsInternalLongIntProc(inst))
	    	        break;
		        inst.GetVariable(v1,1);
		        vName=GlobalVars.GetName(v1,VAR_LIB_FUNCTION);
    		    if (!vName)
                    vName=GlobalVars.GetName(v1,VAR_USER_FUNCTION);
	    	    if (vName)
                    tmp=vName;
		        else
		        {
		            if (inst.operSize1) tmp = String((Word)inst.Data12);
		            else tmp="";
		            tmp += String(inst.Data11);
		        }
		        sprintf(EndOf(LineBuffer),"%s(",(char *)tmp);
		        // For some library procedures, no parameters are passed
		        //    by stack, hence ignore the stack in such cases.    This
		        //    is checked by finding if the previous instruction
		        //    was a PUSH - if not, just ignore the stack.
    		    if (previnst.Instr==PUSH && StackTop)
	    	    {
		            for(i=StackTop-1;i>=0;i--) if (UseAsFnParms[i]) break;
		            if (i>=0)
		            {
    		            sprintf(EndOf(LineBuffer),"%s",(char *)Stack[i]);
	    	            i--;
		                for(;i>=0;i--)
			                if (UseAsFnParms[i])
                			    sprintf(EndOf(LineBuffer),",%s",(char *)Stack[i]);
		            }
		        }
		        // Append the address of the instruction 'CALL Proc' to
		        //    the pending line buffer, so that this address tag can
		        //    be used to locate this statement in case it has to be
		        //    deleted from the pending lines array.
		        // Don't terminate LineBuffer with a ';' because pending
		        //    lines should not be terminated now, only while printing.
		        sprintf(EndOf(LineBuffer),")");
		        tmp = toString(";") + String((int)InstAddr) + ";";
		        Regs[REG_AL] = tmp + LineBuffer;
		        Regs[REG_AX] = tmp + LineBuffer;
		        Regs[REG_DX] = tmp + toString("HIWORD(") + LineBuffer + ")";
		        SetRegModified2(REG_AL);
		        SetRegModified2(REG_AX);
		        SetRegModified2(REG_DX);
		        tmp1 = tmp + LineBuffer;
		        strcpy(LineBuffer,(char *)tmp1);
		        CheckAndAddPending(LineBuffer);

		        // Some functions give a RET/RETF xxxx while returning
		        //    so that there is no need to do a stack unwind by the
		        //    caller.    Check for such conditions, and unwind the
		        //    stack by a suitable amount.
		        Word seg,off;
		        if (inst.operSize1==1)
                {
                    seg=inst.Data11;
                    off=inst.Data12;
                }
		        else
                {
                    seg=FP_SEG(Inst);
                    off=inst.Data11;
                }
		        ProcBegin = (Byte FAR *)MK_FP(seg,off);
		        ProcEnd = ProcBegin+GetProcSize(ProcBegin);
		        d.TraceInst(ProcEnd,i);
    		    dec = (int)d.CurInst.Data11 / sizeof(Word);
	    	    for(i=0;i<dec;i++,StackTop--)
		            Stack[StackTop-1] = "";
		        assert(StackTop>=0);		// Stack Underflow?
		        break;
            case LOOP :
                break;
            default   :
                sprintf(errorStr,"Unknown Instruction at address %X",InstAddr);
		        ErrorList.Add(errorStr);
		        break;
        }
        Inst += InstLen;
        ctr += InstLen;
        previnst=inst;
    }
    CheckAndOutput(NULL,How,sList);
    InstAddr = FP_OFF(Inst);
    if (GetLabel(InstAddr,tmp))
    {
        tmp += ":";
        CheckAndOutput(tmp,How,sList);
    }
}
