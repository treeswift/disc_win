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
#ifndef _STRING_H
#define _STRING_H
#include <string.h>
#include <stdlib.h>
#include "Global.h"

#define MAXSTRLEN 256

class String
{
    char Data[MAXSTRLEN];
public:
    String();
    String(Word i);
    String(Byte i);
    String(int i);
    void operator = (Byte i);
    void operator = (Word i);
    void operator = (int i);
    String &Copy(const char *t);
    int IsSubStrPresent(const String &t,int Pos);
    int IsEmpty();
    void    operator += (const String &s);
    void    operator    = (const String &s);
    void    operator += (const char *t);
    void    operator    = (const char *t);
    operator char * ();
    String &operator + (const char *t);
    String &operator + (const String &t);
    int ValueFromHex();
};

#endif