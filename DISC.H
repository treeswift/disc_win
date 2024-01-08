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
#ifndef _DISC_H
#define _DISC_H

#include "Variable.h"
#include "StrList.h"
extern VariableList GlobalVars;		// List of known Global Variables.
extern VariableList LocalVars;		// List of known local variables.
extern StringList   ErrorList;		// List of errors encountered.
extern char errorStr[256];		// Global tmp. string for errors.

// Globals defined in TcWiz0.cpp
extern char Effaddrstr[8][6];
extern char ArithOperators[][2];
extern char CompOperators[][3];
extern char OppCompOperators[][3];

void SetRegisterVariable(char *reg);
int RegisterVariable(int Reg);
char *GetRegVarName(int Reg);

// Declared in Variable.cpp
extern char VariableTypes[][10];
#endif		// Disc.h