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
#include "Disc.h"
#include "Beautify.h"
#include <string.h>
#include <stdio.h>
#include <conio.h>
#define IsHexDigit(x) ((x>='0' && x<='9') || \
                (x>='A' && x<='F') || \
                (x>='a' && x<='f'))
#define IsAlphabet(x) ((x>='a' && x<='z') || (x>='A' && x<='Z') || x=='_')
/*---------------------------------------------------------------
DelRedundantParantheses - Removes a matching "()" if there are no
 operators inside the "()".
Called by DisplayProc only.
---------------------------------------------------------------*/
void CBeautifier::DelRedundantParantheses(char *tStr)
{
    static char OperChars[]="><=+-*/%!~^&|?,;";
    char *str = tStr;
    char *ptr;
    int CheckEntireString=(*tStr!='(');
    // Reach the 1st '('.
    while(*str && *str!='(') str++;
    // EndofString?
    if (!*str) return;
    // else...
    char *StringBegin = str;
    // Check till we reach the corresponding ')'
    do
    {
        str++;
        int OperatorFound=0;

        while(*str && *str!=')')
        {
            // Skip variables like [BP-02] etc.
            if (*str=='[') while(*str!=']') str++;
            // Another "()"? Check that by recursion.
            if (*str=='(')
            {
                // Is it a function call?
                if (!IsHexDigit(*(str-1)) && !IsAlphabet(*(str-1)))
                    DelRedundantParantheses(str);
            }
            // Still the '(' is present? If so, skip till the ')' is reached.
            if (*str=='(')
                while(*str!=')') str++;
            ptr = OperChars;
            while(*ptr) if (*str==*ptr++) break;
            // Did we find a operator match?
            if (*ptr) OperatorFound=1;
            // Check the next character.
            str++;
        }
        if (*str && !OperatorFound)
        {
            // Is it a function call?
            if (IsHexDigit(*(StringBegin-1))) return;
            // Is it any keyword followed by a '(' - like switch(x)?
            if (IsAlphabet(*(StringBegin-1))) return;
            // else...
            strcpy(StringBegin,StringBegin+1);
            str--;
            strcpy(str,str+1);
        }
    }while(*str && CheckEntireString);
}
/*
[-----------------------------------------------------------------------]
[ GetLabel - returns the address of the stringlist containing the given                ]
[    string "Labelxxx;".                ]
[-----------------------------------------------------------------------]
*/
StringList *CBeautifier::GetLabel(StringList *sList,char *labelptr)
{
    int labellen = strlen(labelptr)-1;
    while(sList)
    {
        if (strncmp(sList->Data,labelptr,labellen)==0) return sList;
        sList = sList->Next;
    }
    return NULL;
}
/*----------------------------------------------------------------------
IsJumpingAcrossBlocks - checks if there are any new blocks created and
 not closed between srcList and tgtList.    Used while translating 'goto's
 to do-while loops and in translating 'if' conditions.
----------------------------------------------------------------------*/
int CBeautifier::IsJumpingAcrossBlocks(StringList *srcList,StringList *tgtList)
{
    srcList=srcList->Next;
    int NumUnmatchedBraces=0;
    int OnlyBracesFound=0;
    int i=srcList->GetPos(tgtList);
    while(i--)
    {
        if (srcList->Data[0]=='}')
        {
            NumUnmatchedBraces--;
            OnlyBracesFound=1;
        }
        else
        {
            OnlyBracesFound=0;
            if (srcList->Data[0]=='{') NumUnmatchedBraces++;
        }
        srcList = srcList->Next;
    }
    if (OnlyBracesFound) NumUnmatchedBraces=0;
    return (NumUnmatchedBraces!=0);
}
/*----------------------------------------------------------------------
BeautifyCode - Rearranges the 'if', 'goto' etc. in the form of loops.
----------------------------------------------------------------------*/
void CBeautifier::BeautifyCode(StringList *sList)
{
    StringList *ptr = sList;
    // start replacing if,goto statements with 'while' loops.
    ptr = sList;
    while(ptr)
    {
        if (!(ptr->Data)) break;
        // Conversion code for IF statement & DO-WHILE loop structures.
        if (strncmp(ptr->Data,"if",2)==0)
        {
            // check if the goto statement jumps to a line before the if stat.
            int PosOfIfGoto = sList->GetPos(GetLabel(sList,strstr(ptr->Data,"goto")+5));
            // if so,    this is not a conventional if..else statement.
            // it might be a DO-WHILE statement, checking is done below.
            if (PosOfIfGoto>sList->GetPos(ptr))
            {
                // Check if there is a JMP to from inside to outside a block.
                //    if so,    leave it unchanged.
                if (!IsJumpingAcrossBlocks(ptr,sList->GetList(PosOfIfGoto)))
                {
                    // Now, reverse the condition in the 'if' statement.
                    //    This is a bit difficult to understand.    The actual code is
                    //    generated to JMP if the condition is not satisfied.
                    //    Since we are transforming these conditions into 'if' blocks,
                    //    reversing the condition is necessary.
                    char *str = ptr->Data;
                    int i;
                    while(*str)
                    {
                        // 10 = Number of Jxx instructions decoded in the 'if' statement.
                        for(i=0;i<10;i++)
                            if (*str==CompOperators[i][0] && *(str+1)==CompOperators[i][1])
                break;
                        if (i!=10) break;
                        str++;
                    }
                    *str = OppCompOperators[i][0];
                    *(str+1) = OppCompOperators[i][1];

                    StringList *EndOfIf = sList->GetList(PosOfIfGoto-1);
                    // Is an else clause present?
                    if (strncmp(EndOfIf->Data,"goto",4)==0)
                    {
                        StringList *EndOfElse = GetLabel(sList,EndOfIf->Data+5);
                        // Is this position after the if clause?
                        if (sList->GetPos(EndOfElse)>sList->GetPos(ptr))
                        {
                            EndOfElse->Insert("}");
                            EndOfIf->Replace("}");
                            EndOfIf->Next->Insert("{");
                            EndOfIf->Next->Insert("else");
                        }
                        // else, there this is not the 'else' clause code.
                        else EndOfIf->Next->Insert("}");
                    }
                    else EndOfIf->Next->Insert("}");
                    *strstr(ptr->Data,"goto")=0;
                    ptr->Next->Insert("{");
                }
            }
            // check if it is a DO-WHILE loop structure.
            else
            {
                StringList *DoClause = sList->GetList(PosOfIfGoto);
                // Check if there is a JMP to from inside to outside a block(or back)
                //    if so,    leave it unchanged.
                if (!IsJumpingAcrossBlocks(DoClause,ptr))
                {
                    // Insert in the reverse order.
                    DoClause->Next->Insert("{");
                    DoClause->Next->Insert("do");

                    char whileclause[100];
                    strcpy(whileclause,"}while");
                    // copy the condition part of the if statement.
                    strcat(whileclause,ptr->Data+2);
                    // cut-off the goto part of the if statement.
                    *strstr(whileclause,"goto")=0;
                    strcat(whileclause,";");
                    ptr->Replace(whileclause);
                }
            }
        }
        // Conversion code for the WHILE loop structure.
        else if (strncmp(ptr->Data,"goto",4)==0)
        {
            StringList *PtrOfGotoLabel = GetLabel(ptr,ptr->Data+5);
            StringList *PtrOfIf = PtrOfGotoLabel->Next;
            if (PtrOfIf)
            {
                StringList *PtrOfIfGoto;
                char *tptr = strstr(PtrOfIf->Data, "goto");
                if (tptr)
                {
                    // check if the goto statement jumps to a line after the if stat.
                    int PosOfIfGoto = sList->GetPos(GetLabel(PtrOfIf,tptr+5));
                    // If so,    this is not a while loop
                    if (PosOfIfGoto<sList->GetPos(PtrOfIf))
                    {
                        // check from before the if clause.
                        PtrOfIfGoto = GetLabel(ptr,strstr(PtrOfIf->Data,"goto")+5);
                        // found?
                        if (ptr->Next == PtrOfIfGoto)
                        {
                            char whileclause[100];
                            strcpy(whileclause,"while");
                            // copy the condition part of the if statement.
                            strcat(whileclause,PtrOfIf->Data+2);
                            // cut-off the goto part of the if statement.
                            *strstr(whileclause,"goto")=0;
                            ptr->Replace(whileclause);
                            ptr->Next->Insert("{");
                            PtrOfIf->Replace("}");
                        }
                    }
                }
            }
        }
        ptr = ptr->Next;
    }
    // Now do some beautification work.
    ptr = sList;
    while(ptr)
    {
        if (!ptr->Data) break;
        // trim down unwanted parantheses.
        DelRedundantParantheses(ptr->Data);
        ptr = ptr->Next;
    }
    DeleteUnusedLabels(sList);
}
void CBeautifier::DeleteUnusedLabels(StringList *sList)
{
    int NumUsedLabels;
    StringList *ptr;

    int NumLabelsfound=0;
    NumUsedLabels=0;
    int *LabelArr;
ComForFindAndStore:
    ptr = sList;
    while(ptr && ptr->Data)
    {
        char *str = strstr(ptr->Data,"goto");
        if (str)
        {
            if (NumLabelsfound)
            {
                int lab = atoi(str+10);
                for(int i=0;i<NumUsedLabels;i++)
                    if (LabelArr[i]==lab) break;
                if (i==NumUsedLabels) LabelArr[NumUsedLabels++] = lab;
            }
            else NumUsedLabels++;
        }
        ptr = ptr->Next;
    }
    if (!NumLabelsfound && NumUsedLabels)
    {
        LabelArr = new int[NumUsedLabels];
        assert(LabelArr);
        NumLabelsfound=1;
        NumUsedLabels=0;
        goto ComForFindAndStore;
    }
    ptr = sList;
    while(ptr && ptr->Data)
    {
        StringList *tmp = ptr;
        ptr = ptr->Next;
        if (strncmp(tmp->Data,"Label",5)==0)
        {
            int lab = atoi(tmp->Data+5);
            for(int i=0;i<NumUsedLabels;i++)
                if (LabelArr[i]==lab) break;
            if (i==NumUsedLabels)
                sList->Delete(tmp);
        }
    }
}
StringList *CBeautifier::DisplayList(StringList *sList,FILE *fp,int Indent,int InCase)
{
    while(sList && sList->Data)
    {
        if (sList->Data[0]=='}') return sList;
        else if (InCase)
        {
            if (strncmp(sList->Data,"default",6)==0 ||
                    strncmp(sList->Data,"case",4)==0) return sList;
        }
    PrintLine:

        if (sList->Data)
        {
            /* Prints the address of each line.... */
            /*
            if ((int)sList->addr!=-1)
                fprintf(fp,"% 4X: ",sList->addr);
            else fprintf(fp,"            ");
            */
            for(int i=0;i<Indent;i++) fprintf(fp," ");
            fprintf(fp,"%s\n",sList->Data);

            if (sList->Data[0]=='{')
            {
                sList = DisplayList(sList->Next,fp,Indent+2,0); goto PrintLine;
            }
            else if (strncmp(sList->Data,"case",4)==0)
            {
                sList = DisplayList(sList->Next,fp,Indent+2,1); goto PrintLine;
            }
            else if (strncmp(sList->Data,"default",7)==0)
            {
                sList = DisplayList(sList->Next,fp,Indent+2,1);
                return sList;
            }
            sList = sList->Next;
        }
    }
    return NULL;
}
