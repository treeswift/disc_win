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
#ifndef _PROGRAM_H			// Program.h
#define _PROGRAM_H

#include "Global.h"
class Program
{
    Word CodeSegment;
    Word DataSegment;
    Byte *CodePtr;
    Dword Size;			// In Bytes.
public:
    Program();
    ~Program();
    int Load(char *FileName);
    Byte FAR *GetCodePtr();
    Byte FAR *GetDataPtr();
    Dword GetCodeSize();
};

#endif				// Program.h