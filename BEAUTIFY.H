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
#ifndef _BEAUTIFY_H
#define _BEAUTIFY_H
#include <stdio.h>
#include "StrList.h"
class CBeautifier
{
public:
    void DelRedundantParantheses(char *tStr);
    StringList *GetLabel(StringList *sList,char *labelptr);
    void BeautifyCode(StringList *sList);
    void DeleteUnusedLabels(StringList *sList);
    StringList *DisplayList(StringList *sList,FILE *fp,int Indent=0,int InCase=0);
    int IsJumpingAcrossBlocks(StringList *srcList,StringList *tgtList);
};

#endif		// Beautify.h
