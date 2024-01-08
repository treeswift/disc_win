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
#ifndef _VARIABLE_H
#define _VARIABLE_H

#include "Global.h"
// Enumerations for the known Variable Types.
enum
{
    VAR_CHAR=0,VAR_INT,VAR_LONG,VAR_VOID,
    VAR_LIB_FUNCTION,VAR_USER_FUNCTION,VAR_STRUCT,VAR_UNION,
    VAR_TYPEDEF,VAR_REGISTER,VAR_UNKNOWN
};

class Variable
{
public:
    int Operand;			// Type of operand.
    Word Data1;			// Operand 1. (Value depends on the Type).
    Sword Data2;			// Used for indexing addr. modes.
    operator char *();
    void operator = (Variable &v);
    Variable &operator - (int offset);
    int operator == (Variable &v);
    int SameKind(Variable &v);
    int IsCVariable();		// is this variable a valid 'C' variable?
    Word GetAddress();
};

class VariableList
{
public:
    Byte Type;			// Type of this variable.
    int Size;			// sizeof(this variable).
    char Name[32];		// Name of this variable.
    Variable Data;			// This variable.
    Byte readOnly;		// Is this variable read-only? i.e. can't del
    VariableList *Next;

    VariableList();
    ~VariableList();
    inline void Flush() { VariableList::~VariableList(); }
    void Display();
    void Add(Variable &v,int _type,int _size,char *_name=0,int _readonly=0);
    void Del(Variable &v);
    void MakeAllReadOnly();
    int GetType(Variable &v);
    char *GetName(Variable &v,int _type=-1);
    int IsPresent(Variable &v,int _type=-1);
    int PartOfAnyVariable(Variable &v1,Variable &v2,int &v2Type);
    void NameUnknownVariables();
};

#endif		// Variable.h
