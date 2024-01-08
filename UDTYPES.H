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
#ifndef _UDTYPES_H
#define _UDTYPES_H
#include "Global.h"
#include "Variable.h"	// just for those enumerations

class UDType;
typedef struct
{
public:
    Byte Type;
    UDType *ptr;
    char *Name;
}UDTypeEntry;

class UDType
{
public:
    Byte Type;		// Typedef (or) Struct (or) Union?
    char *Name;
    int Size;
    int NumVars;
    UDTypeEntry *Entries;

    UDType();
    ~UDType();
    int Assign(char *str);
    void operator = (UDType &t);
    void Display(char *);
    void Invalidate();
};

class UDTypeList
{
    UDType Data;
    UDTypeList *Next;
public:
    UDTypeList();
    ~UDTypeList();
    UDType *Add(UDType &t);
    UDType *GetType(char *Name);
    void Display();
};

#endif		// UdTypes.h
