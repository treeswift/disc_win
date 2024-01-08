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
#include <dos.h>
#include <stdio.h>
#include <conio.h>
#include "Program.h"
#include "Disasm.h"
#include "TcWizard.h"
#include "LibWiz.h"

VariableList GlobalVars;		// List of known global variables.
VariableList LocalVars;			// List of known local variables.
StringList     ErrorList;			// List of errors encountered
					//    while traversing thro' the code.
char errorStr[256];			// Global temp. string for errors.

/*
 * Empty wrappers for functions specific to TurboC
 */
void clrscr()
{
}
void gotoxy(int x,int y)
{
    x=y;
}
int wherey()
{
    return 1;
}

void SetRegisterVariable(char *RegName, char *VarName)
{
    int W,Reg;
    for(W=0;W<2;W++)
        for(Reg=0;Reg<8;Reg++)
            if (strcmp(Regstr[W][Reg],RegName)==0)
            {
	            Variable v;
	            v.Operand=REG_DIRECT;
	            v.Data1=W*8+Reg;
	            v.Data2=0;
	            GlobalVars.Add(v,VAR_REGISTER,0,VarName);
	            return;
            }
}
/*
[-----------------------------------------------------------------------]
[ RegisterVariable							]
[-----------------------------------------------------------------------]
[ Returns 1 if the register passed is used as a register variable.	]
[-----------------------------------------------------------------------]
*/
int RegisterVariable(int whichReg)
{
    Variable v;
    v.Operand=REG_DIRECT;
    v.Data1=whichReg;
    v.Data2=0;
    if (GlobalVars.IsPresent(v))
        return 1;
    return 0;
}

char *GetRegVarName(int whichReg)
{
    Variable v;
    v.Operand=REG_DIRECT;
    v.Data1=whichReg;
    v.Data2=0;
    char *t=GlobalVars.GetName(v);
    if (t)
        return t;
    static char t1[10];
    t1[0]='_'; strcpy(t1+1,Regstr[whichReg/8][whichReg%8]);
    return t1;
}
void DisplayErrors()
{
    StringList *ptr = &ErrorList;
    while(ptr && ptr->Data)
    {
        puts(ptr->Data);
        ptr=ptr->Next;
    }
    ErrorList.Flush();
    printf("Press any key to continue...\n");
    getch();
}

main(int argc,char *argv[])
{
    Program prog;
    Dword funcaddr;
    Word seg, off;
	int addr,type;
	Variable v;
	int size;
	char varstr[10];

    char *ExeName=(argc>1)?argv[1]:"TMP.EXE";
    if (prog.Load(ExeName)!=0)
    {
        printf("Unable to load executable %s.\nCheck if you have PREPDISC.EXE in the current directory!\nIf not,build the file \"prepdisc.c\" using TurboC\n",ExeName);
        return 0;
    }

    Byte FAR *ProgAddr = prog.GetCodePtr();
    TcWizard w(ProgAddr,prog.GetCodeSize());
    Byte FAR *MainAddr = w.GetMain();
    if (MainAddr)
    {
        Variable v;
        v.Operand=IMMEDIATE;
        v.Data1=FP_OFF(MainAddr);
        v.Data2=FP_SEG(MainAddr);
        GlobalVars.Add(v,VAR_USER_FUNCTION,0,"main");
    }
    else
        printf("Unable to locate main!\n");
    int endwhile=0;
    clrscr();

    printf("Globals:-\n"); GlobalVars.Display();

    while(!endwhile)
    {
        DisplayErrors();
        printf("\t\t'D'isassembly\n");
        printf("\t\tdis'C'\n");
        printf("\t\t'R'egister variables\n");
        printf("\t\tcompare in 'L'ibrary\n");
        printf("\t\t'A'dd to variable list\n");
        printf("\t\t'G'lobal variables display\n");
        printf("\t\t'F'ull decompilation\n");
        printf("\tPress the key of your choice (ESC to quit)\n");
        switch(getch())
        {
            case 'g' : case 'G' :
            {
	            GlobalVars.Display(); break;
            }
            case 'a' : case 'A' :
            {
	            printf("Local or global variable(L/G)?\n");
	            switch(getch())
	            {
	                case 'l' : case 'L' :
                    {
		                while(1)
		                {
		                    printf("Enter the variable's address : ");
		                    gets(varstr);
		                    if (!varstr[0]) break;
		                    addr=atoi(varstr+2);
		                    printf("Enter the variable type\n");
		                    printf("(1-char, 2-int, 3-long)");
		                    scanf("%d",&type);
		                    v.Operand=INDEXED_WORD;
		                    v.Data1=INDX_BP;
		                    v.Data2=addr;
		                    size=1<<(type-1);
		                    LocalVars.Add(v,type,size);
		                    fflush(stdin);
		                }
		                break;
                    }
	                case 'g' : case 'G' :
                    {
		                printf("Enter the variable's address : ");
		                scanf("%X",&addr);
		                printf("Enter the variable type\n");
		                printf("(1-char, 2-int, 3-long)");
		                scanf("%d",&type);
		                v.Operand=MEMORY;
		                v.Data1=addr;
		                size=1<<(type-1);
		                GlobalVars.Add(v,type,size);
		                fflush(stdin);
		                break;
                    }
	            }
	            break;
            }
            case 'l' : case 'L' :
            {
	            printf("Function address please (in hex): ");
	            long addr;
	            scanf("%lX",&addr);
	            if (addr==0) break;
	            printf("Enter the library file name : ");
	            char LibFileName[80];
	            scanf("%s",LibFileName);

	            Byte FAR *pa;
	            if (addr<(long)65536) pa = (Byte FAR *)MK_FP(FP_SEG(ProgAddr),(unsigned)addr);
	            else pa=(Byte FAR *)addr;
	            TLibWizard tlw;
	            char pname[64];
	            int percent;
	            if (tlw.CheckInLibrary(LibFileName,1,&pa,pname,&percent)==0)
	            {
	                printf("\nMatched with %s at %d.%d%%\n",pname,percent/10,percent%10);
	                Variable v;
	                v.Operand=IMMEDIATE;
	                v.Data1=FP_OFF(pa);
	                v.Data2=FP_SEG(pa);
	                if (percent>950) GlobalVars.Add(v,VAR_LIB_FUNCTION,0,pname);
	            }
	            else
	                printf("\nUnable to load %s\n",LibFileName);
	            break;
            }
            case 'd' : case 'D' :
            {
	            printf("Enter the address from which to disassemble (in hex): ");
	            scanf("%lX",&funcaddr);
	            if (funcaddr==0) break;
	            seg=FP_SEG(ProgAddr);
	            off=(Word)funcaddr;
	            if (funcaddr>off) seg=funcaddr>>16;
	            Disasm d;
	            d.SetCodePtr(ProgAddr);
	            d.SetregIP((Byte FAR *)MK_FP(seg,off) - ProgAddr);
	            do
	            {
	                d.DispPage(10);
	            }while(getch()!=27);
	            break;
            }
            case 'r' : case 'R' :
            {
	            do
	            {
	                puts("Which register is used as variable? (e.g: AX,AH,SI,DI...)\nPress <enter> to stop\n");
	                char regname[10], varname[32];
	                fflush(stdin); gets(regname);
	                if (!regname[0]) break;
	                strupr(regname);
                        puts("What variable name you want to substitute?");
                        fflush(stdin); gets(varname);
	                if (varname[0] != 0)
                            SetRegisterVariable(regname, varname);
                        else
                            SetRegisterVariable(regname, NULL);
	            }while(1);
	            GlobalVars.NameUnknownVariables();
	            break;
            }
            case 'f' : case 'F' :
            {
	            w.DeCompileFull();
	            break;
            }
            case 'c' : case 'C' :
            {
	            printf("Enter the function address : ");
	            scanf("%lX",&funcaddr);
	            if (funcaddr==0) break;
	            seg=FP_SEG(ProgAddr);
	            off=(Word)funcaddr;
	            if (funcaddr>off) seg=funcaddr>>16;
	            // Initialize info about the procedure.
	            Byte FAR *ProcAddr=(Byte FAR *)MK_FP(seg,off);
	            printf("Recurse all(Y/N)?");
	            int recurse=(getch()=='y');
	            w.Pass1(ProcAddr,recurse);
	            w.Flush();
	            clrscr();
	            LocalVars.NameUnknownVariables();
	            printf("\nLocals:-\n"); LocalVars.Display();
	            DisplayErrors();
	            LocalVars.MakeAllReadOnly();
	            w.DisplayProc(ProcAddr);
	            LocalVars.Flush();
	            break;
            }
            default : endwhile=1; break;
        }
    }
    return 0;
}
