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
#include "DisC.h"
#include "LibWiz.h"
#include "Disasm.h"
#include <conio.h>
TLibWizard::TLibWizard()
{
    FuncNames=0;
    FuncOffsets=0;
    Code=0;
    NumFuncs=0;
}

TLibWizard::~TLibWizard()
{
}

int TLibWizard::FindCodeSegment()
{
    Byte *tModule = Module;
    int CodeSegmentIndex=0;
    int Index=0;
    int SegIndex=0;
    // Find which segment is the CODE segment.
    while((tModule-Module)<ModuleSize && (*tModule!=0x8a))      // MODEND
    {
        if (!CodeSegmentIndex && *tModule==0x96)                // LNAMES
        {
            Byte *Mod = tModule;
            int blocksize = *(short int *)(tModule+1);
            tModule+=1+2;
            while(blocksize>1)
            {
                Index++;
                if (strncmp((const char *)tModule+1,"CODE",4)==0)
                    CodeSegmentIndex=Index;
                blocksize -= *tModule+1;
                tModule+=*tModule+1;
            }
            tModule=Mod;
        }
        if (*tModule==0x98)                // SEGDEF
        {
            SegIndex++;
            if (*(tModule+1+2+1+2+1)==CodeSegmentIndex)
                return SegIndex;
        }
        tModule+=3+(*(short int *)(tModule+1));
    }
    return 0;
}

int TLibWizard::GetFuncs()
{
    Byte *tModule = Module;
    int CodeSegment = FindCodeSegment();
    int NumFuncsFound=0;
    int CodeSize;
ComForFindAndStore:
    NumFuncs=0;
    CodeSize = 0;
    while((tModule-Module)<ModuleSize && (*tModule!=0x8a))                // MODEND
    {
        if (*tModule==0x90 && *(tModule+1+2+1)==CodeSegment)                // PUBDEF
        {
            if (NumFuncsFound)
            {
                int FuncNameLen = *(tModule+1+2+1+1);
                if(FuncNameLen>MAX_NAME_LEN)
                    ErrorList.Add("Function Name in library exceeds allocated memory.");
                strncpy((char *)FuncNames+NumFuncs*MAX_NAME_LEN,
                    (const char *)tModule+1+2+1+1+1,FuncNameLen);
                *(FuncNames+NumFuncs*MAX_NAME_LEN+FuncNameLen)=0;

                FuncOffsets[NumFuncs]=*(Word *)(tModule+1+2+1+1+1+FuncNameLen);
            }
            NumFuncs++;
        }
        else if (*tModule==0xA0 && *(tModule+1+2)==CodeSegment)                // LEDATA
        {
            int DataSize = (*(Word *)(tModule+1))-(1+2+1);
            if (NumFuncsFound)
            {
                memcpy(Code+CodeSize,tModule+1+2+1+2,DataSize);
            }
            CodeSize += DataSize;
        }
        tModule+=3+(*(short int *)(tModule+1));
    }
    if (!NumFuncsFound && NumFuncs)
    {
        NumFuncsFound=1;
        tModule = Module;
        // allocate for Code & names & offsets of the functions.
        if (Code)
            free(Code);
        Code = (Byte *)malloc(CodeSize + NumFuncs*MAX_NAME_LEN + NumFuncs*2);
        if (!Code)
        {
            NumFuncs=0;
            return 1;
        }
        FuncNames = Code + CodeSize;
        FuncOffsets = (Word *)(FuncNames + NumFuncs*MAX_NAME_LEN);
        goto ComForFindAndStore;
    }

    for(int i=0;i<NumFuncs-1;i++)
    {
        for(int j=i;j<NumFuncs;j++)
        {
            if (FuncOffsets[i]>FuncOffsets[j])
            {
                char tmp[MAX_NAME_LEN];
                int tOffset;
                tOffset=FuncOffsets[i];
                FuncOffsets[i]=FuncOffsets[j];
                FuncOffsets[j]=tOffset;
                strcpy(tmp,(const char *)FuncNames+i*MAX_NAME_LEN);
                strcpy((char *)FuncNames+i*MAX_NAME_LEN,(const char *)FuncNames+j*MAX_NAME_LEN);
                strcpy((char *)FuncNames+j*MAX_NAME_LEN,(const char *)tmp);
            }
        }
    }
    return 0;
}

/*--------------------------------------------------------------
EndOfLibrary - returns 1 if the MSLEND block has been reached in
 the given file.
--------------------------------------------------------------*/
int TLibWizard::EndOfLibrary(FILE *fp)
{
    Byte tmp;
    fread(&tmp,1,1,fp);
    if (tmp==0xF1)
        return 1;                // MSLEND.
    fseek(fp,-1,SEEK_CUR);
    return 0;
}

/*--------------------------------------------------------------
LoadNextModule - Loads the next module from the library file and
 initializes the Function names, function offsets and code.
Use EndOfLibrary() to check if the end has been reached, before
 calling this function.    This function does not do the checking.
--------------------------------------------------------------*/
int TLibWizard::LoadNextModule(FILE *fp)
{
    TLibWizard::~TLibWizard();
    long pos = ftell(fp);
    Byte BlockType;
    short int BlockSize;
    // find the current module size.
    do
    {
        fread(&BlockType,1,1,fp);
        fread(&BlockSize,2,1,fp);
        fseek(fp,BlockSize,SEEK_CUR);
    }while(BlockType!=0X8A);
    // Reach out for the beginning of the next module;
    do
    {
        fread(&BlockType,1,1,fp);
    }while(BlockType!=0x80 && BlockType!=0xF1);                // THEADR or MSLEND
    // Position exactly at the beginning of next module.
    fseek(fp,-1,SEEK_CUR);

    ModuleSize = ftell(fp)-pos;
    Module = (Byte *)malloc(ModuleSize);
    if (!Module) return 1;

    fseek(fp,pos,SEEK_SET);
    // read current module.
    fread(Module,ModuleSize,1,fp);

    // Now load the functions and return the error code if any.
    int retval= GetFuncs();
    free(Module);
    return retval;
}

int TLibWizard::CompareFuncs(Byte FAR *Proc1,Byte FAR *Proc2)
{
    Disasm d1,d2;
    int InstLen1,InstLen2;
    InstInfo *i1,*i2;
    d1.SetCodePtr(Proc1);
    d2.SetCodePtr(Proc2);
    int NumInst=0,MatchInst=0;
    i1 = &d1.CurInst;
    i2 = &d2.CurInst;
    do
    {
        d1.TraceInst(Proc1,InstLen1);
        d2.TraceInst(Proc2,InstLen2);
        Proc1+=InstLen1;
        Proc2+=InstLen2;
        // If we encounter any segment override, don't take it into consideration.
        //    Just skip the instructon and proceed.
        if (i1->Instr==SEGMENT) { d1.TraceInst(Proc1,InstLen1); Proc1+=InstLen1; }
        if (i2->Instr==SEGMENT) { d2.TraceInst(Proc2,InstLen2); Proc2+=InstLen2; }

        if (i1->Instr==i2->Instr &&
                i1->Operand1==i2->Operand1 &&
                i1->Operand2==i2->Operand2)
            MatchInst++;
        NumInst++;
    }while(i1->Instr!=RET && i2->Instr!=RET);
    // return the matching percentage, upto 2nd decimal place.
    return ((long)MatchInst * 1000) / NumInst;
}

void TLibWizard::Dump(Byte FAR *Proc1,Byte FAR *Proc2)
{
    Disasm d1,d2;
    int lineno=0;
    String Instr;
    int Inst1Len,Inst2Len;
    int whichProcOver=0;
    do
    {
        Instr = d1.GetAsm(Proc1,Inst1Len);
        int row=wherey();
        gotoxy(1,row);
        printf("%s",(char *)Instr);

        Instr = d2.GetAsm(Proc2,Inst2Len);
        gotoxy(41,row);
        printf("%s",(char *)Instr);

        printf("\n");
        d1.TraceInst(Proc1,Inst1Len);
        d2.TraceInst(Proc2,Inst2Len);
        Proc1+=Inst1Len; Proc2+=Inst2Len;
        lineno++;
        if (lineno%23==0)
        {
            printf("--More--");
            getch();
        }
        if (d1.CurInst.Instr==RET)
            whichProcOver |= 1;
        if (d2.CurInst.Instr==RET)
            whichProcOver |= 2;
    }while(whichProcOver<3);
}

int TLibWizard::CheckInLibrary(
                char *LibFileName,
                int NumGivenProcs,
                Byte FAR **Procs,
                char *ProcNames,// Each name must be allocated 64bytes
                int *CmpPercent)
{
    if (NumGivenProcs==0) return 0;
    FILE *fp = fopen(LibFileName,"rb");
    if (!fp) return 1;

    for(int i=0;i<NumGivenProcs;i++)
        CmpPercent[i]=-1;
    Disasm d;
    while(!EndOfLibrary(fp))
    {
        if (LoadNextModule(fp)!=0)
        {
            ErrorList.Add("Insufficient memory to load next module from library.");
            break;
        }
        for(int p=0;p<NumFuncs;p++)
        {
            Byte *LibraryProc = Code + FuncOffsets[p];
            if (kbhit() && getch()==27) goto endofcompare;
            printf("Comparing with %s                     \r",FuncNames+p*MAX_NAME_LEN);

            for(i=0;i<NumGivenProcs;i++)
            {
                int percent=CompareFuncs(Procs[i],LibraryProc);
                if (percent>CmpPercent[i])
                {
                    CmpPercent[i]=percent;
                    strcpy(ProcNames+i*64,(const char *)FuncNames+p*MAX_NAME_LEN);
                }
            }
        }
        for(i=0;i<NumGivenProcs;i++)
            if (CmpPercent[i]<990) break;
        if (i==NumGivenProcs) break;
    }
endofcompare:
    if (Code) free(Code);
    Code=0;
    fclose(fp);
    return 0;
}
