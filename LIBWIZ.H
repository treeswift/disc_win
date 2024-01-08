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
#ifndef _LIBWIZ_H
#define _LIBWIZ_H

#include <stdio.h>
#include "Global.h"
class TLibWizard
{
#define MAX_NAME_LEN 40
    Byte *Module;			// Points to the Current Module Data.
    long ModuleSize;            // Length of this module.

    Byte *FuncNames;		// Names of functions in current module.
    Word *FuncOffsets;		// Offsets of these fns. in the CODE segment.
    int NumFuncs;			// No of functions in current module.
    Byte *Code;			// Code of the functions in the CODE segment.

    int FindCodeSegment();
    int GetFuncs();
    int EndOfLibrary(FILE *fp);
    int InitLibrary(FILE *fp);
    int LoadNextModule(FILE *fp);
    int CompareFuncs(Byte FAR *Proc1,Byte FAR *Proc2);
    void Dump(Byte FAR *Proc1,Byte FAR *Proc2);
public:
    TLibWizard();
    ~TLibWizard();
    int CheckInLibrary(char *LibFileName,int NumGivenProcs,
	    Byte FAR *Procs[],char ProcNames[],int CmpPercent[]);
};

#endif		// LibWizard.h