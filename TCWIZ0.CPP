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
//File : TcWiz0.Cpp
//Implementation of the TcWizard class member functions.
//Continues in TcWiz1.Cpp
//
#include <dos.h>
#include <stdio.h>
#include <conio.h>
#include <assert.h>
#include "Disc.h"
#include "TcWizard.h"
#include "Disasm.h"
#include "Beautify.h"
#include "LibWiz.h"

char ArithOperators[][2]  = {"","+","-","&","|","^"};
char CompOperators[][3]   = {"> ",">=","< ","<=","==","!=","> ",">=","< ","<="};
char OppCompOperators[][3]={"<=","< ",">=","> ","!=","==","<=","< ",">=","> "};

TcWizard::TcWizard(Byte FAR *Ptr /* = 0 */,Dword size /* = 0 */)
{
    SetCodePtr(Ptr,size);
    StackTop = 0;
    PendingTop = 0;

    BlockTable = NULL;
    NumBlocks = 0;

    TracingFunctionCall=0;
}

TcWizard::~TcWizard()
{
    FlushAll();
}

void TcWizard::FlushAll()
{
    Flush();

    if (BlockTable) free(BlockTable);
    BlockTable=NULL;
    NumBlocks=0;

    LocalVars.Flush();
}

/*--------------------------------------------------------------
Flush - resets the object for another trial with the same
 procedure.    If we need to reset all info regarding the
 procedure, call FlushAll.
--------------------------------------------------------------*/
void TcWizard::Flush()
{
    int i=0; for(;i<8+8;i++)
        ((char *)Regs[i])[0]=0;

    for(i=0;i<NUM_STACK_LINES;i++)
    {
        ((char *)Stack[i])[0]=0;
        UseAsFnParms[i]=0;
    }
    StackTop=0;

    ((char *)CompareVar.Operand1)[0]=0;

    for(i=0;i<NUM_PENDING_LINES;i++)
        ((char *)PendingLines[i])[0]=0;
    PendingTop=0;

    TracingFunctionCall=0;
}

void TcWizard::SaveState(TcWizardState &t)
{
    int i;
    for(i=0;i<8+8+4;i++)
    {
        if (Regs[i].IsEmpty())
            t.Regs[i]=NULL;
        else
        {
            t.Regs[i] = (char *)malloc(strlen((char *)Regs[i])+1);
            if (t.Regs[i]==NULL)
            {
                sprintf(errorStr,"Insufficient memory to save internal state at %X",InstAddr);
                ErrorList.Add(errorStr);
            }
            else
                strcpy(t.Regs[i],(char *)Regs[i]);
        }
    }
    t.StackTop=StackTop;
    t.PendingTop=PendingTop;
    t.TracingFunctionCall=TracingFunctionCall;
    t.InstAddr=InstAddr;
}

void TcWizard::RestoreState(TcWizardState &t)
{
    for(int i=0;i<8+8+4;i++)
        if (t.Regs[i])
            strcpy((char *)Regs[i],t.Regs[i]);
    StackTop=t.StackTop;
    PendingTop=t.PendingTop;
    TracingFunctionCall=t.TracingFunctionCall;
    InstAddr=t.InstAddr;
}

void TcWizard::DestroyState(TcWizardState &t)
{
    for(int i=0;i<8+8+4;i++)
        if (t.Regs[i]) free(t.Regs[i]);
}

void TcWizard::ClearPending()
{
    PendingTop = 0;
}
/*
[-----------------------------------------------------------------------]
[ FlushPending                                                          ]
[-----------------------------------------------------------------------]
[ Stores the pending string into the passed parameter and returns if    ]
[    any more strings are to be processed.                              ]
[-----------------------------------------------------------------------]
*/
int TcWizard::FlushPending(String &t,Word &iAddr)
{
    if (!PendingTop)
        return 0;
    int i = 0;
    char *Pos1 = strchr((char *)PendingLines[i],';');
    if (!Pos1)
        return 0;
    char *Pos2 = strchr(Pos1 + 1,';');
    assert(Pos2);                // Make sure we have a matching pair of ';'.

    int j=Pos2 - Pos1 + 1;
    String tmp; tmp=Pos1;
    ((char *)tmp)[j] = 0;                // Copy only the ";XXXX;" portion.
    iAddr=atoi((char *)tmp+1);           // Give the instruction address to the caller.

    // Now delete the ";XXXX;" portion from the source String.
    strcpy(Pos1,Pos2+1);
    t = PendingLines[i];
    PendingTop--;
    for(i=0;i<PendingTop;i++)
        PendingLines[i]=PendingLines[i+1];
    return 1;
}

int TcWizard::AddPending(char *str)
{
    assert(PendingTop<=NUM_PENDING_LINES);
    int pos = PendingTop;
    PendingLines[PendingTop++] = str;
    return pos;
}

/*
[-----------------------------------------------------------------------]
[ DeletePending                                                         ]
[-----------------------------------------------------------------------]
[ Deletes the pending line containing the string which is passed.       ]
[ Useful in deleting entries for function calls, whose return values    ]
[ have been used in subsequent instructions.                            ]
[-----------------------------------------------------------------------]
*/
void TcWizard::DeletePending(String &Key)
{
    int i=0;
    char *Pos1 = strchr((char *)Key,';');
    if (!Pos1)
        return;
    char *Pos2 = strchr(Pos1 + 1,';');
    assert(Pos2);                // Make sure we have a matching pair of ';'.

    i=Pos2 - Pos1 + 1;
    String tmp; tmp=Pos1;
    ((char *)tmp)[i] = 0;                // Copy only the ";XXXX;" portion.

    // Now delete the ";XXXX;" portion from the source String.
    strcpy(Pos1,Pos2+1);

    for(i=0;i<PendingTop;i++)
        if (PendingLines[i].IsSubStrPresent(tmp,0))
            break;
    if (i<PendingTop)
    {
        for(;i<PendingTop;i++)
            PendingLines[i] = PendingLines[i+1];
        PendingTop--;
    }
}

void TcWizard::SetCodePtr(Byte FAR *Ptr,Dword size)
{
    CodePtr = Ptr;
    Size = size;
    Regs[REG_CS] = String((Word)FP_SEG(Ptr));
    Regs[REG_ES] = String((Word)(FP_SEG(GetDataSegment())));
    Regs[REG_DS] = String((Word)(FP_SEG(GetDataSegment())));
    Regs[REG_SS] = Regs[REG_DS];

}

Byte FAR *TcWizard::GetMain()
{
    Dword i;
    Byte FAR *Ptr = CodePtr;
    int n=0;
    for(i=0;i<Size;i++)
    {
        if (*(Word FAR *)Ptr == 0x36FF)                // Opcode for PUSH [mem].
        {
            Ptr += 4;
            n++;
        }
        else if (*Ptr == 0x9a || *Ptr == 0xe8)
        {
            if (n>=3) break;
            else Ptr++;
        }
        else { n=0; Ptr++; }
    }
    Word segment=0,offset=0;
    if (n>=3)
    {
        if (*Ptr == 0x9a)
        {
            segment = *(Word FAR *)(Ptr+3);
            offset    = *(Word FAR *)(Ptr+1);
        }
        else
        {
            segment = FP_SEG(CodePtr);
            offset    = (Word)((Ptr - CodePtr) + 3 + (*(int FAR *)(Ptr + 1)));
        }
    }
    return (Byte FAR *)MK_FP(segment,offset);
}

Byte FAR *TcWizard::GetDataSegment()
{
    return (Byte FAR *)MK_FP(*(Word FAR *)(CodePtr+1),0);
}

Dword TcWizard::GetProcSize(Byte FAR *ProcBegin)
{
    Disasm d;
    return d.GetProcSize(ProcBegin);
}

void TcWizard::TranslateTrickyInsts(InstInfo &inst)
{
    switch(inst.Instr)
    {
    case XOR : // XOR val,val==>MOV val,0
    case OR  : // OR    val,val==>CMP val,0
        if (inst.Operand1==inst.Operand2 &&
                inst.Data11==inst.Data21 &&
                inst.Data12==inst.Data22)
        {
            inst.Instr = (inst.Instr==XOR) ? MOV : CMP;
            inst.Operand1 = inst.Operand1;
            inst.Data11 = inst.Data11;
            inst.Data21 = inst.Data21;
            inst.Operand2 = IMMEDIATE;
            inst.Data21 = 0;
            inst.Data22 = 0;
        }
        break;
    }
}
/*
[---------------------------------------------------------------------------]
[ SkipProcInit                                                              ]
[---------------------------------------------------------------------------]
[ Finds howmany 'PUSH's have been given in the beginning of the procedure   ]
[    and subtracts an equal number of 'POP's at the end of the procedure    ]
[    so that the stack is balanced properly.                                ]
[ The actual procedure size is stored and returned in the CodeSize variable.]
[ RETURN VALUE : No of bytes to skip at the beginning of the procedure.     ]
[---------------------------------------------------------------------------]
*/
Dword TcWizard::SkipProcInit(Byte FAR *ProcBegin,Dword &ProcSize)
{
    Disasm d;
    d.SetCodePtr(CodePtr);
    Byte FAR *Inst = ProcBegin;
    Dword ctr =0;
    int InstLen = 0;
    int InitOver=0;
    int NumPushes=0;
    // Skip the initial PUSH BP; MOV BP,SP; & any number of PUSHs following.
    do
    {
        Inst += InstLen;
        ctr += InstLen;
        d.TraceInst(Inst,InstLen);
        switch(d.CurInst.Instr)
        {
        case PUSH :
            NumPushes++;
            break;                // Any number of pushes.
        case MOV  :
            if (!(d.CurInst.Operand1 == REG_DIRECT &&
                    d.CurInst.Data11 == REG_BP))                // Mov bp,sp
                InitOver=1;
                break;
        case SUB  :
            if (!(d.CurInst.Operand1 == REG_DIRECT &&
                    d.CurInst.Data11 == REG_SP))                // Sub sp,val
                InitOver=1;
            break;
        default   :
            InitOver=1;
        }
    }while(!InitOver);

    int TotalPops=0;
    // Find the total number of POPs after this initial code.
    InstLen = 0;
    InitOver=0;
    do
    {
        Inst += InstLen;
        d.TraceInst(Inst,InstLen);
        switch(d.CurInst.Instr)
        {
            case POP  : TotalPops++; break;
            case RET  : InitOver=1;
        }
    }while(!InitOver);

    // Once again start from the beginning, till we reach the instruction
    //    from where No. of pops=No. of initial pushes, so that the actual
    //    procedure ends at that point.
    InstLen = 0;
    Inst = ProcBegin + ctr;                // Skip the initial PUSHs and MOV BP,SP.
    do
    {
        Inst += InstLen;
        d.TraceInst(Inst,InstLen);
        // Check if we have reached the MOV SP,BP instruction.
        if (d.CurInst.Instr==MOV &&
                (d.CurInst.Operand1==REG_DIRECT && d.CurInst.Data11==REG_SP) &&
                (d.CurInst.Operand2==REG_DIRECT && d.CurInst.Data21==REG_BP))
        // If so,    this is the end of the procedure.
            break;
        if (d.CurInst.Instr==POP) TotalPops--;
    }while(TotalPops>=NumPushes);
    // Now we are in the first POP corresponding to the initial PUSHs.

    ProcSize = Inst - (ProcBegin + ctr);
    // Return the initial no. of bytes to skip.
    return ctr;
}

int sortFunc(const void *a,const void *b)
{
    if (((CBlock *)a)->Address > ((CBlock *)b)->Address) return 1;
    if (((CBlock *)a)->Address < ((CBlock *)b)->Address) return -1;
    return 0;
}

/*
[-----------------------------------------------------------------------]
[ IsSwitchStatement - Returns 1 if the given address corresponds to a   ]
[    switch statement.    Finds this out from the BlockTable array.This ]
[    array must be initialised before, by calling the Pass1 function.   ]
[-----------------------------------------------------------------------]
*/
int TcWizard::IsSwitchStatement(Word Address)
{
    for(int i=0;i<NumBlocks;i++)
        if ((BlockTable[i].Type==C_SWITCH ||
                 BlockTable[i].Type==C_SWITCH_ORDERED) &&
                 BlockTable[i].Address==Address)
            return BlockTable[i].Type;
    return 0;
}

/*
[-----------------------------------------------------------------------]
[ Pass1                                                                 ]
[-----------------------------------------------------------------------]
[ Go thro' the procedure and find the addresses given for JMPs and put  ]
[    in the BlockTable.                                                 ]
[ Also finds the library functions called in this procedure.            ]
[ Also find the addresses of the SWITCH statements                      ]
[ Logic to find switch statements:                                      ]
[                ---1st kind of switch.---                              ]
[     switch(x) compiles as                                             ]
[       MOV CX, NumSwitchEntries   // excluding the DEFAULT Clause      ]
[       MOV BX, AddressOfSwitchTable                                    ]
[ [-- label1:                                                           ]
[ |     CS:                // or whatever segment it might be           ]
[ |     MOV Reg,[BX]                                                    ]
[ |     CMP Reg,x                                                       ]
[ | [-- JZ label2                                                       ]
[ | |   ADD BX,2                                                        ]
[ [---  LOOP label1                                                     ]
[   |   JMP AddressOf_DEFAULT_Clause                                    ]
[   [-label2:                                                           ]
[       CS:                // or whatever segment                       ]
[       JMP [BX+Disp]      // Disp = size of the Values table in the    ]
[                          //    Switch Table                           ]
[       // The address table starts at SwitchTable+Disp                 ]
[              ---2nd kind of switch.(Ordered-Switch)---                ]
[     switch(x)                                                         ]
[     {                                                                 ]
[         case 0 :                                                      ]
[         case 1 :                                                      ]
[         case 2 :                                                      ]
[         ...                                                           ]
[         case MAX:                                                     ]
[     }                                                                 ]
[                compiles as shown below.                               ]
[       MOV Reg,x                                                       ]
[       CMP Reg,MAX                                                     ]
[       JA    DefaultAddress                                            ]
[       SHL Reg,1                                                       ]
[       CS:                                                             ]
[       JMP [Reg+SwitchTableAddress]                                    ]
[ ProcBegin - Beginning of the procedure.                               ]
[ Recurse - 1 if we can recursively search all procedures for functions ]
[             called and variables used and setup all the related lists.]
[-----------------------------------------------------------------------]
*/
void TcWizard::Pass1(Byte FAR *ProcBegin,int Recurse)
{
    Flush();
    static int SwitchFormat[]={ CMP,JA,SHL,SEGMENT,JMP };
    Byte FAR *Inst;
    Disasm d;
    d.SetCodePtr(CodePtr);
    Dword ProcSize;
    Dword ctr = SkipProcInit(ProcBegin,ProcSize);
    // This initialization of ProcEndAddress is a must, in pass1.
    // this prevents from tracing instructions after the end of the
    // given procedure.
    ProcEndAddress = ProcBegin + ctr + ProcSize;
    Inst = ProcBegin + ctr;
    Byte FAR *tInstBegin = Inst;
    int NumBlocksFound=0;
    if (BlockTable) free(BlockTable);
    Byte FAR **ProcTable=NULL;
    char *ProcNames;
    int *ProcPercent;
    int NumProcsCalled;
    Byte FAR *tInst;
    int tInstLen;
ComForFindAndStore:
    ctr=0;
    NumBlocks = 0;
    NumProcsCalled=0;
    while(ctr<ProcSize)
    {
        int i;
        int InstLen;
        d.TraceInst(Inst,InstLen);
        Inst += InstLen;
        ctr += InstLen;
        InstInfo inst = d.CurInst;
        TranslateTrickyInsts(inst);
        switch(inst.Instr)
        {
            // forming a list of functions called by this procedure.
            case CALL: if (NumBlocksFound)
                 {
                     Word seg=FP_SEG(ProcBegin);
                     if (inst.operSize1)
                         seg=inst.Data12;
                     Byte FAR *p=(Byte FAR *)MK_FP(seg,inst.Data11);
                     Variable v;
                     v.Operand=IMMEDIATE;
                     v.Data1=FP_OFF(p);
                     v.Data2=FP_SEG(p);
                     // if the procedure has already been scanned, skip it.
                     if (GlobalVars.IsPresent(v))
                         break;
                     // Check if an entry has already been made
                     for(int i=0;i<NumProcsCalled;i++)
                         if (ProcTable[i]==p)
                             break;
                     // no?
                     if (i==NumProcsCalled)
                     {
                         ProcTable[NumProcsCalled++]=p;
                     }
                 }
                 else NumProcsCalled++;
                 break;
            // checking for the Ordered-Switch.
            case CMP : tInst = Inst;
                 for(i=1;i<5;i++)
                 {
                     d.TraceInst(tInst,tInstLen);
                     if (d.CurInst.Instr!=SwitchFormat[i])
                         break;
                     tInst+=tInstLen;
                 }
                 if (i==5)
                 {
                     if (NumBlocksFound)
                     {
                         BlockTable[NumBlocks].Type = C_SWITCH_ORDERED;
                         BlockTable[NumBlocks].Address = FP_OFF(Inst-InstLen);
                     }
                     NumBlocks++;
                 }
                 break;
            case RET :
                ctr = ProcSize;
                break;
            // Loop means that we got the address of a SWITCH statement.
            case LOOP:
                if (NumBlocksFound)
                {
                    BlockTable[NumBlocks].Type = C_SWITCH;
                    BlockTable[NumBlocks].Address = inst.Data11;

                    // Find the length of the switch table data, and skip it.
                    d.TraceInst(Inst,InstLen);        // JMP Default_label_address
                    Inst += InstLen;
                    ctr += InstLen;
                    d.TraceInst(Inst,InstLen);        // CS:
                    Inst += InstLen;
                    ctr += InstLen;
                    d.TraceInst(Inst,InstLen);        // JMP [BX+Length_of_table]
                    Inst += InstLen;
                    ctr += InstLen;
                    InstLen = d.CurInst.Data12*2;
                    Inst += InstLen;
                    ctr += InstLen;
                }
                NumBlocks++;
                break;
//                JA, JAE, JB, JBE, JZ, JNZ, JG, JGE, JL, JLE
//                Others normally do not occur in C Programs. Hence ignored.
            case JMP :
                if (inst.Operand1!=IMMEDIATE)
                    break;
            case JA : case JAE : case JB : case JBE :
            case JL : case JG  : case JZ :
            case JLE: case JGE : case JNZ:
                if (NumBlocksFound)
                {
                    // Check if the address is already present in the table.
                    for(register int i=0;i<NumBlocks;i++)
                        if (BlockTable[i].Type==C_GOTO && \
                            BlockTable[i].Address==inst.Data11)
                            break;
                    // If not, store it.
                    if (i<=NumBlocks)
                    {
                        BlockTable[NumBlocks].Type = C_GOTO;
                        BlockTable[NumBlocks].Address = inst.Data11;
                        NumBlocks++;
                    }
                }
                else NumBlocks++;
                break;
        }
    }
    if (!NumBlocksFound)
    {
        if (NumBlocks)
        {
            BlockTable = (CBlock *)malloc(sizeof(CBlock)*NumBlocks);
            if (!BlockTable)
            {
                ErrorList.Add("Not enough memory in Pass1!");
                exit(1);
            }
            NumBlocksFound=1;
        }
        if (NumProcsCalled)
        {
            ProcTable = (Byte FAR **)malloc(
                sizeof(Byte FAR *)*NumProcsCalled +
                sizeof(int)*NumProcsCalled +
                sizeof(char)*64*NumProcsCalled);
            if (!ProcTable)
            {
                ErrorList.Add("Not enough memory in Pass1!");
                exit(1);
            }
            ProcPercent=(int *)(ProcTable+NumProcsCalled);
            ProcNames=(char *)(ProcPercent+NumProcsCalled);
            NumBlocksFound=1;
        }
        if (NumBlocksFound)
        {
            Inst=tInstBegin;
            goto ComForFindAndStore;
        }
    }
    qsort(BlockTable,NumBlocks,sizeof(CBlock),sortFunc);
    // Now find the library functions.
    TLibWizard tlw;
    if (tlw.CheckInLibrary("cs.lib",NumProcsCalled,ProcTable,ProcNames,ProcPercent)==0)
    {
        Variable v;
        for(int i=0;i<NumProcsCalled;i++)
        {
            v.Operand=IMMEDIATE;
            v.Data1=FP_OFF(ProcTable[i]);
            v.Data2=FP_SEG(ProcTable[i]);
            if (ProcPercent[i]>950)
                GlobalVars.Add(v,VAR_LIB_FUNCTION,0,ProcNames+i*64);
            else
            {
                GlobalVars.Add(v,VAR_USER_FUNCTION,0,NULL);
                sprintf(errorStr,"Function at address %Fp not found in library",ProcTable[i]);
                ErrorList.Add(errorStr);
                if (Recurse) Pass1(ProcTable[i],Recurse);
            }
        }
    }
    if (ProcTable) free(ProcTable);
    // Go thro' the procedure once and register all variables whose datatypes
    //    can be found from the code structure.
    // ** Do it only if we aren't going recursively.    If so, the caller will
    // **    do this checking.
    TranslateBlock(tInstBegin,ProcSize,SEPARATE_LINES,NULL);
}

void TcWizard::Output(const char *Statement, int How, StringList *sList)
{
    String tmp,tmp1,tmp2;
    if (TracingFunctionCall) How=COMMA_EXPRESSIONS;
    Word iAddr;
    while(FlushPending(tmp2,iAddr))
    {
        switch(How)
        {
            case SEPARATE_LINES        :
                tmp = tmp2+";";
                if (sList) sList->Add(tmp,iAddr);
                break;
            case COMMA_EXPRESSIONS : tmp1 += tmp2; tmp1 += ","; break;
        }
    }
    if (Statement)                // Pass NULL to do only Flushing of pending
    {                //    lines.
        switch(How)
        {
            case SEPARATE_LINES        :
                tmp = "";
                tmp += Statement;
                if (sList) sList->Add(tmp,InstAddr); break;
            case COMMA_EXPRESSIONS : tmp1 += Statement;
                char *ptr = (char *)tmp1;
                int len = strlen(ptr);
                if (ptr[len]==';') ptr[len]=',';
                else tmp1 += ",";
                break;
        }
    }
    if (How==COMMA_EXPRESSIONS)
    {
        if (TracingFunctionCall)
        {
            int i=StackTop;
            if (((char *)Stack[i])[0] == 0) Stack[i] = "(";
            Stack[i] += tmp1;
        }
        else
        {
            if (sList) sList->Add(tmp1);
        }
    }
}

void TcWizard::DisplayProc(Byte FAR *ProcBegin)
{
    StringList sList;
    Dword ProcSize;
    Dword ctr = SkipProcInit(ProcBegin,ProcSize);
    InstAddr=FP_OFF(ProcBegin+ctr);
    Variable v;
    v.Operand=IMMEDIATE;
    v.Data1=FP_OFF(ProcBegin);
    v.Data2=FP_SEG(ProcBegin);
    char FuncName[35];
    if (FuncName) strcpy(FuncName,GlobalVars.GetName(v));
    else strcpy(FuncName,(char *)String((Word)v.Data1));
    strcat(FuncName,"()");
    Output(FuncName,SEPARATE_LINES,&sList);
    Output("{",SEPARATE_LINES,&sList);

    // this initialization is a must in displayproc and pass1
    ProcEndAddress=ProcBegin+ctr+ProcSize;

    TranslateBlock(ProcBegin+ctr,ProcSize,SEPARATE_LINES,&sList);
    Output("}",SEPARATE_LINES,&sList);
    CBeautifier cb;
    printf("\nRestructure code(Y/N)?\n");
    int i=getch();
    if (i=='y' || i=='Y') cb.BeautifyCode(&sList);
    FILE *fp=fopen("_disc.c","a");
    if (!fp) fp=fopen("_disc.c","w");
    if (!fp) printf("Unable to open _disc.c\n");
    else
    {
        cb.DisplayList(&sList,fp);
        printf("Written to file _DISC.C.\n");
        fclose(fp);
    }
}

/*
[-----------------------------------------------------------------------]
[ CheckForTernaryOperCode - must be called only by TranslateBlock.      ]
[-----------------------------------------------------------------------]
[ Logic: Ternary operator compiles as                                   ]
[                 CMP val1,val2                                         ]
[     [--- Jxx label1                                                   ]
[     |                 ...                                             ]
[     |[-- JMP label2                                                   ]
[     [--                label1:                                        ]
[      |                 ...                                            ]
[      [- label2:                                                       ]
[          possible assignment code etc.                                ]
[          ##End of ternary operator code.                              ]
[   Find the 1st JMP from CMP, find the address given in the JMP(this is]
[                the end of the block).                                 ]
[   Find if any registers are modified and not used in the block        ]
[        (possibly used after the block(i.e. are you using the result of]
[          the ternary operator). If so, then 'tis a ternary operator.  ]
[ Code - points to the instruction after the Jxx to be checked.         ]
[ ConditionStr - points to the condition eg. (i==1)                     ]
[-----------------------------------------------------------------------]
*/
Byte FAR *TcWizard::CheckForTernaryOperCode(Byte FAR *Code,char *ConditionStr)
{
    Disasm d;
    d.SetCodePtr(CodePtr);
    Byte FAR *Inst = Code;
    int InstLen;
    // Find the JMP and the end of the block from that instruction.
    // If we have another ternary operator inside this block, we must skip
    //    the JMP of the second block, so that we find the JMP of this first
    //    ternary operator correctly.
    int NumJumpsToSkip=0;
    Byte FAR *Fragment1Begin=0; Word Fragment1Size=0;
    Byte FAR *Fragment2Begin=0; Word Fragment2Size=0;
    Word ctr=0,BlockSize=CompareVar.EndAddress-FP_OFF(Code);
    while(ctr<BlockSize)
    {
        d.TraceInst(Inst,InstLen);
        if (d.CurInst.Instr==JMP)
        {
            if (!NumJumpsToSkip)
            {
                // Now Inst points to the JMP instruction.
                Fragment1Begin = Code;
                Fragment2Begin = Inst+InstLen;
                Byte FAR *Fragment2End = (Byte FAR *)MK_FP(d.CurInst.Data21,d.CurInst.Data11);
                Fragment1Size = Inst - Fragment1Begin;
                Fragment2Size = Fragment2End - Fragment2Begin;
                break;
            }
            else NumJumpsToSkip--;
        }
        else if (d.CurInst.Instr==CMP) NumJumpsToSkip++;
        Inst += InstLen;
        ctr += InstLen;
    }
    // Error in searching for the JMP?
    if (ctr==BlockSize)
        return Code;
    if (Fragment1Size<=0 || Fragment2Size<=0)
        return Code;
    int i;
    // Now find which registers are modified in the first code fragment.
    Byte RegModified[8+8];                // No SegmentRegisters are checked here.
    Byte RegUsed[8+8];
    Byte RegModifiedLater[8+8];
    Byte RegUsedLater[8+8];
    for(i=0;i<16;i++)
    {
        RegModified[i]=0; RegUsed[i]=0;
        RegModifiedLater[i]=0; RegUsedLater[i]=0;
    }

    TcWizardState CurState;
    SaveState(CurState);

    FindRegsUsed(Fragment1Begin,Fragment1Size,RegModified,RegUsed);
    RestoreState(CurState);
    FindRegsUsed(Fragment2Begin+Fragment2Size,15,RegModifiedLater,RegUsedLater);
    RestoreState(CurState);
    // Check if any of the registers were modified, but not used later.
    // *** Check if RegModified[i]==1, since RegModified[i]==2 if there
    // ***    was a function call, whose return value type is not known.
    // *** Ignore such Modifications to the register values here.
    for(i=0;i<16;i++)
        if ((RegModified[i]==1 && !RegUsed[i]) &&
                (RegModifiedLater[i]!=1 && RegUsedLater[i]))
        break;
    // No such registers? Well, then this is not a ternary operator code.
    if (i==16)
        return Code;

    String t; t=ConditionStr;
    String tRegs[16];
    // Giving SEPARATE_LINES or COMMA_EXPRESSIONS here doesn't matter, because
    //    sList = NULL here.
    TranslateBlock(Fragment2Begin,Fragment2Size,SEPARATE_LINES,NULL);
    for(i=0;i<16;i++) tRegs[i] = t + " ? (" + Regs[i] + ") : (";
    RestoreState(CurState);
    TranslateBlock(Fragment1Begin,Fragment1Size,SEPARATE_LINES,NULL);
    // Now change only those registers used in the ternary operator code.
    for(i=0;i<16;i++)
        if ((RegModified[i]==1 && !RegUsed[i]) &&
                (RegModifiedLater[i]!=1 && RegUsedLater[i]))
            tRegs[i] += Regs[i] + ")";
    RestoreState(CurState);
    for(i=0;i<16;i++)
        if ((RegModified[i]==1 && !RegUsed[i]) &&
                (RegModifiedLater[i]!=1 && RegUsedLater[i]))
        {
            // support for long-int returning ternary operators.
            if (i==REG_DX || i==REG_CX) Regs[i]="HIWORD("; else Regs[i]="";
            Regs[i] += tRegs[i];
            if (i==REG_DX || i==REG_CX) Regs[i]+=")";
        }

    DestroyState(CurState);
    return Fragment2Begin+Fragment2Size;
}

int TcWizard::IsEqualClosingParentheses(String &t)
{
    char *ptr = (char *)t;
    int OpeningParentheses=0,ClosingParentheses=0;
    while(*ptr)
    {
        if (*ptr=='(') OpeningParentheses++;
        else if (*ptr==')') ClosingParentheses++;
        ptr++;
    }
    return (OpeningParentheses==ClosingParentheses);
}
