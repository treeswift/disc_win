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
#ifndef _GLOBAL_H		// Global.h
#define _GLOBAL_H

typedef signed char SByte;
typedef signed short int Sword;
typedef unsigned char Byte;
typedef unsigned short int Word;
typedef unsigned int Dword;
typedef signed int SDword;
#define TRUE  1
#define FALSE 0

extern void clrscr();
extern void gotoxy(int x,int y);
extern int wherey();

#ifdef WIN32

#include <windows.h>

extern Word gCodeSegment;
extern Byte *gCodePtr;

#define FP_SEG(fptr) (gCodeSegment)
#define FP_OFF(fptr) (((Byte *)fptr - gCodePtr))
#define MK_FP(seg, off) (gCodePtr + off)

#ifdef _CRT_NONSTDC_DEPRECATE
#define getch _getch
#define itoa _itoa
#define kbhit _kbhit
#define strupr _strupr
#define gets(s) gets_s(s, sizeof(s))
#endif

#else

#define FAR far

#endif

#define EndOf(str) (str+strlen(str))

#endif				// Global.h