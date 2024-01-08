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
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <assert.h>
#include "InstInfo.h"
#include "Variable.h"
char VariableTypes[VAR_UNKNOWN+1][10] = {"char","int","long","void","proc",\
                "proc","struct","union","typedef","int","unknown"};
int Variable::SameKind(Variable &v)
{
    switch(Operand)
    {
        case OPER_UNKNOWN:
            return 0;
        case REG_DIRECT:
        case IMMEDIATE:
        case MEMORY:
            return Operand==v.Operand;
        // for other cases,...
        default:
            return Operand==v.Operand && Data1==v.Data1;
    }
}
int Variable::IsCVariable()
{
    switch(Operand)
    {
        case REG_DIRECT:
            return 1; // for register variables.
        case INDEXED_BYTE:
        case INDEXED_WORD:
            if (Data1!=INDX_BP)
                return 0;
        case MEMORY:
        case IMMEDIATE:
            return 1;
        default:
            return 0;
    }
}
void Variable::operator = (Variable &v)
{
    Operand=v.Operand;
    Data1=v.Data1;
    Data2=v.Data2;
}
Variable &Variable::operator - (int offset)
{
    static Variable v;
    v=*this;
    switch(Operand)
    {
        case REG_DIRECT:
            break;
        case MEMORY:
            v.Data1-=offset;
            break;
        case INDEXED_BYTE:
        case INDEXED_WORD:
            if (v.Data1==INDX_BP)
                v.Data2-=offset;
            break;
        default:
            assert("Address of variable"!="Supported mode");
    }
    return v;
}
int Variable::operator == (Variable &v)
{
    return (Operand==v.Operand && Data1==v.Data1 && Data2==v.Data2);
}
Word Variable::GetAddress()
{
    switch(Operand)
    {
        case MEMORY:
            return Data1;
        case INDEXED_BYTE:
        case INDEXED_WORD:
            if (Data1==INDX_BP)
                return Data2;
            return 0;
        case REG_DIRECT:
            return 0;
        default: assert("Address of variable"!="Supported mode");
    }
    return 0;
}
Variable::operator char *()
{
    InstInfo inst;
    inst.SetVariable(*this,2);
    String tt;
    static char tname[10];
    tname[0]=0;
    switch(Operand)
    {
        case MEMORY:
        case INDEXED_BYTE:
        case INDEXED_WORD:
                strcpy(tname,(char *)inst.GetOperand2(NULL,NULL));
                break;
        case IMMEDIATE      : // variable is containing address of a function.
                tt=(Word)Data2;
                strcpy(tname,(char *)tt);
                strcat(tname,":"); tt=(Word)Data1;
                strcat(tname,(char *)tt);
                break;
    }
    return tname;
}
VariableList::VariableList()
{
    Data.Operand=OPER_UNKNOWN;
    Next=NULL;
}
VariableList::~VariableList()
{
    Data.Operand=OPER_UNKNOWN;
    if (Next) delete Next;
    Next=NULL;
}
void VariableList::Display()
{
    VariableList *ptr=this;
    while(ptr && ptr->Data.Operand!=OPER_UNKNOWN)
    {
        printf("%s is %s of type %s\n",(char *)ptr->Data,ptr->Name,VariableTypes[ptr->Type]);
        ptr=ptr->Next;
    }
}
void VariableList::Add(Variable &v,int _type,int _size,char *_name,int _readonly)
{
    if (!v.IsCVariable()) return;
    VariableList *ptr=this;
    while(ptr->Data.Operand!=OPER_UNKNOWN)
    {
        // If the same variable is present, just replace the type, name, etc.
        if (ptr->Data==v)
        {
            if (ptr->readOnly) return;
            break;
        }
        if (ptr->Next==NULL)
        {
            ptr->Next = new VariableList;
            assert(ptr);
            ptr = ptr->Next;
            break;
        }
        ptr=ptr->Next;
    }
    ptr->Data=v;
    ptr->Type=_type;
    ptr->Size=_size;
    if (_name) strcpy(ptr->Name,_name);
    else strcpy(ptr->Name,"");
    readOnly=_readonly;
}
int VariableList::GetType(Variable &v)
{
    VariableList *ptr=this;
    while(ptr!=NULL && ptr->Data.Operand!=OPER_UNKNOWN)
    {
        if (ptr->Data==v) return ptr->Type;
        ptr=ptr->Next;
    }
    return VAR_UNKNOWN;
}
void VariableList::Del(Variable &v)
{
    VariableList *ptr=this;
    VariableList *parent=this;
    if (Data==v)
    {
        if (readOnly) return;
        if (Next)
        {
            memcpy(this,Next,sizeof(VariableList));
            ptr=ptr->Next;
        }
        else
        {
            Data.Operand=OPER_UNKNOWN; return;
        }
    } else ptr=ptr->Next;
    do
    {
        while(ptr!=NULL && ptr->Data.Operand!=OPER_UNKNOWN)
        {
            if (ptr->Data==v)
            {
                if (ptr->readOnly) return;
                break;
            }
            parent=ptr;
            ptr=ptr->Next;
        }
        if (ptr!=NULL && ptr->Data.Operand!=OPER_UNKNOWN)
        {
            parent->Next=ptr->Next;
            ptr->Next=NULL;
            delete ptr;
            ptr=parent->Next;
        }
    }while(ptr!=NULL && ptr->Data.Operand!=OPER_UNKNOWN);
}
char *VariableList::GetName(Variable &v,int _type)
{
    VariableList *ptr=this;
    while(ptr!=NULL && ptr->Data.Operand!=OPER_UNKNOWN)
    {
        if (ptr->Data==v)
        {
            // check Type only if requested to.
            if (_type>=0 && _type!=Type) goto checknext;
            if (strcmp(ptr->Name,"")==0) return NULL;
            return ptr->Name;
        }
checknext:
        ptr=ptr->Next;
    }
    return NULL;
}
int VariableList::IsPresent(Variable &v,int _type)
{
    VariableList *ptr=this;
    while(ptr!=NULL && ptr->Data.Operand!=OPER_UNKNOWN)
    {
        if (ptr->Data==v)
        {
            // check Type only if requested to.
            if (_type>=0 && _type!=Type) continue;
            return 1;
        }
        ptr=ptr->Next;
    }
    return 0;
}
void VariableList::MakeAllReadOnly()
{
    VariableList *ptr=this;
    while(ptr!=NULL && ptr->Data.Operand!=OPER_UNKNOWN)
    {
        ptr->readOnly=1;
        ptr=ptr->Next;
    }
}
int VariableList::PartOfAnyVariable(Variable &v1,Variable &v2,int &v2Type)
{
    VariableList *ptr=this;
    while(ptr && ptr->Data.Operand!=OPER_UNKNOWN)
    {
        if (ptr->Data.SameKind(v1))
        {
            Word addr2 = ptr->Data.GetAddress();
            Word addr1 = v1.GetAddress();
            // does the given variable's address fall in the range of this variable's
            //    address range?
            addr1+=1000; addr2+=1000;                // 1000 = BP(assumed).
            if (addr1==addr2 || (addr2<=addr1 && addr2+ptr->Size>addr1))
            {
                v2=ptr->Data;
                v2Type=ptr->Type;
                return 1;
            }
        }
        ptr=ptr->Next;
    }
    return 0;
}
void VariableList::NameUnknownVariables()
{
    char prefix[3];
    VariableList *ptr=this;
    int Counters[VAR_UNKNOWN];  // VAR_UNKNOWN must be the last index
                                // in the enum list, hence has the
                                // size of the list.
    for(int i=0;i<VAR_UNKNOWN;i++)
        Counters[i]=1;
    prefix[1]='_'; prefix[2]=0;

    while(ptr && ptr->Data.Operand!=OPER_UNKNOWN)
    {
        if (strcmp(ptr->Name,"")==0)
        {
            switch(ptr->Data.Operand)
            {
                // Entry for any known function.
                case IMMEDIATE   : prefix[0]='g'; break;
                case MEMORY      : prefix[0]='g'; break;
                // for register variables.
                case REG_DIRECT  : prefix[0]='r'; break;
                case REG_INDIRECT:
                case INDEXED_BYTE:
                case INDEXED_WORD:
                    if (ptr->Data.Data1==INDX_BP)
                        prefix[0]=(ptr->Data.Data2<=0)?'l':'p';
                    break;
            }
            strcpy(ptr->Name,prefix);
            strcat(ptr->Name,VariableTypes[ptr->Type]);
            strcat(ptr->Name,"_");
            itoa(Counters[ptr->Type],EndOf(ptr->Name),10);
            Counters[ptr->Type]++;
        }
        else
        {
            // check for similar existing entries like l_int_1, and increment
            //    the appropriate counter.
            for(int i=0;i<VAR_UNKNOWN;i++)
                if (ptr->Name[1]=='_' &&
                     strncmp(ptr->Name+2,VariableTypes[i],strlen(VariableTypes[i]))==0)
                     Counters[i]++;
        }
        ptr=ptr->Next;
    }
}