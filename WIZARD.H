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
#include "Global.h"
#include "MyString.h"
class CWizard
{
public:
    Byte FAR *CodePtr;
    Dword Size;

    void operator = (CWizard &t)
    {
        CodePtr=t.CodePtr;
        Size = t.Size;
    }
    virtual void SetCodePtr(Byte FAR *Ptr,Dword size) = 0;
    virtual Byte FAR *GetMain() = 0;
    virtual Byte FAR *GetDataSegment() = 0;
    virtual void DisplayProc(Byte FAR *ProcPtr) = 0;
};
