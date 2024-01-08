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
#include "StrList.h"

StringList::StringList()
{
    Data = NULL;
    Next = NULL;
    addr = -1;
}
StringList::~StringList()
{
    if (Data) delete[] Data;
    Data=NULL;
    if (Next) delete Next;
    Next=NULL;
}
StringList *StringList::GetList(int Pos)
{
     StringList *ptr = this;
     while(Pos--)
         ptr = ptr->Next;
     return ptr;
}
int StringList::GetPos(StringList *s)
{
     StringList *ptr = this;
     int pos=0;
     do
     {
         if (ptr==s) return pos;
         ptr = ptr->Next;
         pos++;
     }while(ptr);
     return -1;
}
void StringList::Replace(const char *t)
{
    if (Data) delete[] Data;
    Data = NULL;
    Add(t,addr);
}
void StringList::Add(const char *t,int _addr)
{
    if (Data)
    {
        if (!Next) Next = new StringList;
        Next->Add(t,_addr);
    }
    else
    {
        Data = new char[strlen(t)+1];
        assert(Data);
        strcpy(Data,t);
        addr=_addr;
    }
}
/*-------------------------------------------------------------
Insert - inserts the given string in the current position and
 pushes everything else down by one unit.
*** Data in 'this' stringlist is replaced by the new one.
-------------------------------------------------------------*/
void StringList::Insert(const char *t)
{
    if (Data)
    {
        StringList *tmp = Next;
        Next = new StringList;
        Next->Add(Data,addr);
        Next->Next = tmp;
        delete[] Data;
        Data=NULL;
    }
    Add(t);
}
void StringList::Delete(StringList *s)
{
    assert(s!=this);
    StringList *ptr = this;
    while(ptr->Next!=s) ptr = ptr->Next;
    ptr->Next = s->Next;
    s->Next = NULL;
    delete s;
}