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
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include "Program.h"

Word gCodeSegment;
Byte *gCodePtr;

Program::Program()
{
    CodeSegment = 0;
    DataSegment = 0;
}
Program::~Program()
{
    if (CodePtr)
        free(CodePtr);
    CodePtr = 0;
    gCodePtr = 0;
    CodeSegment = 0;
    DataSegment = 0;
}
Byte FAR *Program::GetCodePtr()
{
    return (Byte FAR *)CodePtr;
}
Dword Program::GetCodeSize()
{
    return Size;
}
Byte FAR *Program::GetDataPtr()
{
    printf("\n\nGetDataPtr called!! \n\n");
    return 0;
}
int Program::Load(char *FileName)
{
    if (CodeSegment)
        Program::~Program();

    int rval;
    if ((rval = _spawnl(_P_WAIT, "prepdisc.exe", "prepdisc.exe", FileName, NULL)) == -1)
        return 1;

    FILE *fp;
    fp = fopen("Disc_mem.dmp", "rb");
    if (fp == NULL)
        return 2;
    fseek(fp, 0, SEEK_END);
    Size = ftell(fp)-2;
    CodePtr = gCodePtr = (Byte *)malloc(Size);
    fseek(fp, 0, SEEK_SET);
    fread(&CodeSegment, 2, 1, fp);
    fread(CodePtr, Size, 1, fp);
    fclose(fp);

    return 0;
}