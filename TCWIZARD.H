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
#ifndef _TCWIZARD_H		// TcWizard.h
#define _TCWIZARD_H

#include "Wizard.h"
#include "MyString.h"
#include "StrList.h"
#include "InstInfo.h"
#include "Variable.h"

#define Register(W,R) (W*8 + R)
#define SegmentReg(R) (16 + R)

typedef struct
{
    String Operand1,Operand2;
    Word EndAddress;
}CompareStruct;

typedef struct
{
    Byte Type;
    Word Address;
}CBlock;

typedef struct
{
    int NumClauses;
    Byte FAR *SwitchTable;
    Word DefaultAddress;
    Byte DeleteAfterUse;
}SwitchInfo;

// enumerations for the CBlock.Type member.
enum { C_UNKNOWN=0, C_GOTO, C_SWITCH, C_SWITCH_ORDERED };

class TcWizard : public CWizard
{
    enum { SEPARATE_LINES, COMMA_EXPRESSIONS };
    typedef struct
    {
        char *Regs[8+8+4];
        int StackTop;
        int PendingTop;
        int TracingFunctionCall;
        Word InstAddr;
    }TcWizardState;
    #define NUM_PENDING_LINES 20
    #define NUM_STACK_LINES 10
protected:
    String Regs[8+8+4];			// Strings to store the contents of
					// Byteregs + Wordregs + Segregs.

    String Stack[NUM_STACK_LINES];	// Stack for Procedure calls.
    char UseAsFnParms[NUM_STACK_LINES];	// Used by 'long-int' conversion code.
    int StackTop;

    CompareStruct CompareVar;

    String PendingLines[NUM_PENDING_LINES];// Stack for pending o/p lines.
    int PendingTop;

    CBlock *BlockTable;			// Table of addresses of JMPs,
    int NumBlocks;			//    SWITCHs etc.

    int TracingFunctionCall;		// BOOL variable to know if we are
					//    tracing a fn call or not.

    Byte FAR *ProcEndAddress;		// Convenience variable for checking.
    Word InstAddr;			// Used by 'Output' function.

    void TranslateTrickyInsts(InstInfo &inst);
    String &GetReg(int whichReg);		// Checks the contents and returns it.
    int AddPending(char *str);
    void DeletePending(String &Key);
    void ClearPending();
    int FlushPending(String &t,Word &iAddr);
    Byte FAR * CheckForTernaryOperCode(Byte FAR *Code,char *ConditionStr);
    int IsEqualClosingParentheses(String &t);
    void TranslateBlock(Byte FAR *BlockBegin,Sword BlockSize,int How,StringList *sList,Byte *RegModified,Byte *RegUsed);
    void TranslateBlock(Byte FAR *BlockBegin,Sword BlockSize,int How,StringList *sList);
    void FindRegsModified(Byte FAR *BlockBegin,Sword BlockSize,Byte *RegModified);
    void FindRegsUsed(Byte FAR *BlockBegin,Sword BlockSize,Byte *RegModified,Byte *RegUsed);
    Dword SkipProcInit(Byte FAR *ProcBegin,Dword &ProcSize);
    Dword GetProcSize(Byte FAR *ProcBegin);
    int GetLabel(Word Address,String &s);
    StringList *GetLabel(StringList *sList,char *labelstr);
    Byte FAR *FindInstruction(Byte FAR *Inst,Byte Instruc,InstInfo *inst=NULL,int *LenPtr=NULL);
    int IsReturnStatement();

    // Macros for convenience.
    #define GetOperand1(inst) inst.GetOperand1(Regs)
    #define GetOperand2(inst) inst.GetOperand2(Regs)
    #define ClearSwitchInfo(SwInfo) SwInfo.NumClauses=0
    #define InSwitchStatement(SwInfo) SwInfo.NumClauses
    void FillSwitchInfo(Byte FAR *Inst,InstInfo &inst,SwitchInfo &SwInfo);
    Byte FAR *DecodeSwitchStatement(SwitchInfo &SwInfo,int How,StringList *sList,Word BlockEndAddress);
    int IsSwitchStatement(Word Address);

    // Output functions.
    void Indent(int amount = 4);
    void UnIndent(int amount=4);
    void Output(const char *Statement, int How, StringList *sList);

    // Functions for long-int checking & understanding.
    int IsLongInt(Variable &v1);
    int IsFirstWordOfLongInt(Variable &v1);
    int IsInternalLongIntProc(InstInfo &inst);
    // Functions handling internal long-int procedure calls like multiply etc.
    void LongIntMultiply();
    void LongIntDivide(char *Oper);	// for both divide and mod.
    void LongIntShift(char *Oper);	// for both left & right shift.

    // operations on the current state of the wizard.
    void SaveState(TcWizardState &t);
    void RestoreState(TcWizardState &t);
    void DestroyState(TcWizardState &t);

    // To find the datatypes of the variables used
    void AddVar(Variable &v,int type,int size);
    void DelVar(Variable &v);
    void FindAndRegisterDataTypes(InstInfo &inst);
public:
    TcWizard(Byte FAR *Ptr,Dword size);
    ~TcWizard();
    void Flush();		// resets the registers, stack etc.& not the Block
			//    tables and variable list etc.
    void FlushAll();	// resets everything, including LocalVars list.

    void SetCodePtr(Byte FAR *Ptr,Dword size);
    Byte FAR *GetMain();
    Byte FAR *GetDataSegment();
    void Pass1(Byte FAR *ProcBegin,int Recurse=0);	// Call before calling DisplayProc.
    void DisplayProc(Byte FAR *ProcBegin);
    void DeCompileFull();
};

#endif				// TcWizard.h