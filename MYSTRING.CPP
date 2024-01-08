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
#include "MyString.h"

#define PREFIX_0X

String &String::operator + (const String &t)
{
    static String str;
    str.Copy(Data);
    strcat(str.Data,t.Data);
    return str;
}
String &String::operator + (const char *t)
{
    static String str;
    str.Copy(Data);
    strcat(str.Data,t);
    return str;
}
String::String()//char *tData)
{
    Data[0] = 0;
//    if (tData) *this = tData;
}
String::String(Word i)
{
    *this = i;
}
String::String(Byte i)
{
    *this=i;
}
String::String(int i)
{
    *this=i;
}
void String::operator = (Byte i)
{
    char tmp[3];
    itoa(i,tmp,16);
    strupr(tmp);
    Data[0] = 0;
#ifdef PREFIX_0X
    strcat(Data, "0x");
#endif
    strcat(Data,"00");
    strcpy(Data+2-strlen(tmp),tmp);
}
void String::operator = (Word i)
{
    char tmp[6];
    itoa(i,tmp,16);
    strupr(tmp);
    Data[0] = 0;
#ifdef PREFIX_0X
    strcat(Data, "0x");
    strcat(Data,"0000");
    strcpy(Data+6-strlen(tmp),tmp);
#else
    strcat(Data,"0000");
    strcpy(Data+4-strlen(tmp),tmp);
#endif
}
void String::operator = (int i)
{
    itoa(i,Data,10);
}
String &String::Copy(const char *t)
{
    *this = t;
    return *this;
}
int String::IsSubStrPresent(const String &t,int Pos)
{
    return (strncmp(Data+Pos,t.Data,strlen(t.Data))==0);
}
int String::IsEmpty()
{
    return !Data[0]; 
}
void    String::operator += (const String &s)
{
    strcat(Data,s.Data); 
}
void    String::operator    = (const String &s)
{
    strcpy(Data,s.Data); 
}
void    String::operator += (const char *t)
{
    strcat(Data,t); 
}
void    String::operator    = (const char *t)
{
    strcpy(Data,t); 
}
String::operator char * ()
{
    return Data; 
}
int String::ValueFromHex()
{
    int val;
    sscanf(Data,"%X",&val);
    return val;
}