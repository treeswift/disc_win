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
#ifndef _STRINGLIST_H
#define _STRINGLIST_H

#include <Assert.h>
#include "MyString.h"

class StringList
{
public:
    Word addr;
    char *Data;
    StringList *Next;

    StringList();
    ~StringList();
    inline void Flush() { StringList::~StringList(); }
    StringList *GetList(int Pos);
    int GetPos(StringList *s);
    void Replace(const char *t);
    void Add(const char *t,int _addr=-1);
    void Insert(const char *t);
    void Delete(StringList *s);
};

#endif		// StringList.h