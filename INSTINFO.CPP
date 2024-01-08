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
#include <assert.h>
#include "InstInfo.h"
#include "Wizard.h"
#include "DisC.h"

extern char Effaddrstr[8][6];
extern char Regstr[2][8][3];
void InstInfo::Clear()
{
    Instr = UNKNOWN;
    operSize1 = operSize2 = 0;
    Operand1 = Operand2 = OPER_UNKNOWN;
    Data11 = Data12 = 0;
    Data21 = Data22 = 0;
}
void InstInfo::SwapOperands()
{
    int o1;
    int t1;
    Word w1;
    Sword s1;
    o1 = operSize1; t1 = Operand1; w1 = Data11; s1 = Data12;
    operSize1 = operSize2; Operand1 = Operand2;
    Data11 = Data21; Data12 = Data22;
    operSize2 = o1; Operand2 = t1; Data21 = w1; Data22 = s1;
}
String InstInfo::GetOperand(int oper,String *Regs,int getName)
{
    String str,tmp,tmp1;
#define toStr(s) tmp1.Copy(s)
    int val;
    char *vName=0;
    Variable v;
    int whichReg, whichReg1;

    GetVariable(v,oper);
    switch(v.Operand)
    {
    case IMMEDIATE :
        str = String((Word)v.Data1);
        break;
    case REG_DIRECT :
        if (RegisterVariable(v.Data1))
            str=GetRegVarName(v.Data1);
        else
        str=Regs[v.Data1];
        break;
    case REG_INDIRECT :
        tmp="";
ComForIndexed:
        whichReg=-1;
        whichReg1=-1;
        switch(v.Data1)
        {
            case INDX_BX_SI : whichReg=REG_BX; whichReg1=REG_SI; break;
            case INDX_BX_DI : whichReg=REG_BX; whichReg1=REG_DI; break;
            case INDX_BX : whichReg = REG_BX; break;
            case INDX_SI : whichReg = REG_SI; break;
            case INDX_DI : whichReg = REG_DI; break;
            case INDX_BP :
                // for local variables like [BP-02] etc.
                if (getName)
                    vName=LocalVars.GetName(v);
                if (vName)
                    str=vName;
                else
                {
                    str = toStr("[") + Effaddrstr[v.Data1] + tmp + "]";
                }
                break;
            default: str = toStr("*(") + Effaddrstr[v.Data1] + tmp + ")";
        }
        if (whichReg!=-1)
        {
            if (RegisterVariable(whichReg))
            {
                str=GetRegVarName(whichReg);
                str = toStr("*(") + str + tmp + ")";
            }
            else
            {
                str = toStr("*(") + Regs[whichReg];
                if (whichReg1!=-1) str += toStr(" + ") + Regs[whichReg1];
                str += tmp + ")";
            }
        }
        break;
    case MEMORY :
        if (getName)
        vName=GlobalVars.GetName(v);
        if (vName)
        str=vName;
        else
        {
            str = toStr("["); str += v.Data1 ; str+= "]";
        }
        break;
    case INDEXED_BYTE :
        val = v.Data2;
        tmp += (val<0)?"-":"+";
        tmp += (val<0)?(Byte)(256-val):(Byte)val;
        goto ComForIndexed;
    case INDEXED_WORD :
        val = v.Data2;
        tmp += (val<0)?"-":"+";
        tmp += (val<0)?(Byte)(256-val):(Byte)val;
        goto ComForIndexed;
    }
    return str;
}
Variable &InstInfo::GetVariable(Variable &t,int whichVar)
{
    if (whichVar==1)
    {
        t.Operand=Operand1;
        t.Data1=Data11;
        t.Data2=Data12;
    }
    else
    {
        t.Operand=Operand2;
        t.Data1=Data21;
        t.Data2=Data22;
    }
    return t;
}
Variable &InstInfo::SetVariable(Variable &t,int whichVar)
{
    if (whichVar==1)
    {
        Operand1=t.Operand;
        Data11=t.Data1;
        Data21=t.Data2;
    }
    else
    {
        Operand2=t.Operand;
        Data21=t.Data1;
        Data22=t.Data2;
    }
    return t;
}
