#include "UDTypes.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>

UDTypeList udt;
UDType::UDType()
{
    NumVars=0;
    Size=0;
    Name=NULL;
    Entries=NULL;
}
void UDType::Invalidate()
{
    NumVars=0;
    Size=0;
    Name=NULL;
    Entries=NULL;
}
void UDType::Display(char *n=0)
{
    switch(Type)
    {
        case VAR_STRUCT : printf("struct %s\n{\n",Name); break;
        case VAR_UNION  : printf("union %s\n{\n",Name); break;
        case VAR_TYPEDEF: printf("typedef "); break;
    }
    for(int i=0;i<NumVars;i++)
    {
        switch(Entries[i].Type)
        {
            case VAR_VOID : printf("void %s\n",Entries[i].Name); break;
            case VAR_CHAR : printf("char %s\n",Entries[i].Name); break;
            case VAR_INT  : printf("int    %s\n",Entries[i].Name); break;
            case VAR_LONG : printf("long %s\n",Entries[i].Name); break;
            case VAR_UNION:
            case VAR_STRUCT:Entries[i].ptr->Display(Entries[i].Name); break;
        }
    }
    if (Type!=VAR_TYPEDEF)
    {
        printf("}");
        if (n) printf("%s",n);
    }
    printf("\n");
}
UDType::~UDType()
{
    if (Name) free(Name);
    if (Entries) free(Entries);
    NumVars=0;
    Size=0;
    Name=NULL;
    Entries=NULL;
}
void UDType::operator =(UDType &t)
{
    memcpy(this,&t,sizeof(UDType));
}
int next(char *str,char *s,char *_tmp=0)
{
    char tmp[100];
    sscanf(str,"%s",tmp);
    int i=0;
    while(tmp[i]!=0 && tmp[i]!=';' && tmp[i]!='{' && tmp[i]!='}' && tmp[i]!=',')
        i++;
    tmp[i]=0;
    if (_tmp!=0) strcpy(_tmp,tmp);
    return (strcmp(tmp,s)==0);
}
char *skipto(char *str,int c,int c1=0)
{
    while(*str!=c && *str!=c1 && *str!=0) str++;
    if (*str!=0) str++;
    return str;
}
char *spc(char *str)
{
    while((*str>=0x09 && *str<=0x0d) || *str==0x20) str++;
    return str;
}
char *getdecl(char *_str,UDTypeEntry &t)
{
    if (!*_str) return NULL;

    static char *declstr=0;
    char *str;
    t.Type=VAR_UNKNOWN;
    t.ptr=NULL;
    char tmp[40];
    // check for previous declarations
    if (!declstr)
        str=_str;
    else
        str=declstr;
    // skip the type modifiers.
    str=spc(str);
    if (next(str,"signed")) str+=6;
    else if (next(str,"unsigned")) str+=8;
    // find the datatype.
    str=spc(str);
    next(str,"",tmp);
    if (strcmp(tmp,"void")==0) { t.Type=VAR_VOID; str+=4; }
    else if (strcmp(tmp,"short")==0) { t.Type=VAR_INT; str+=5; }
    else if (strcmp(tmp,"int")==0) { t.Type=VAR_INT; str+=3; }
    else if (strcmp(tmp,"char")==0) { t.Type=VAR_CHAR; str+=4; }
    else if (strcmp(tmp,"long")==0)
    {
        t.Type=VAR_LONG;
        str+=4;
        str=spc(str);
        if (next(str,"int")) str+=3;
    }
    else if (strcmp(tmp,"struct")==0)
    {
        char *tstr=str;
        str+=6; str=spc(str);
        UDType *utype=NULL;
        // is it a structure defined inside this declaration?
        if (*str=='{')
        {
            UDType newtype;
            newtype.Assign(tstr);
            if (newtype.NumVars!=0)
	            utype=udt.Add(newtype);
            newtype.Invalidate();
            // skip till the corresponding '}'.
            int i=1;
            do
            {
	            str=skipto(str+1,'}','{');
	            if (*(str-1)=='{') i++;
	            else i--;
            }while(i);
        }
        else
        {
            str=spc(str);
            // get the struct name.
            next(str,"",tmp);
            utype=udt.GetType(tmp);
            // skip the struct name.
            str+=strlen(tmp);
        }
        if (utype) { t.Type=VAR_STRUCT; t.ptr=utype; }
    }
    else if (strcmp(tmp,"union")==0)
    {
        str+=5; str=spc(str);
        UDType *utype=udt.GetType(str);
        if (utype) { t.Type=VAR_UNION; t.ptr=utype; }
    }
    else
    {
        // if control comes here, then the declaration is of the form
        // "typedef unsigned size_t", or something else, where int,char,long,
        // struct,union have not been given.    The default datatype used is 'int'.
        t.Type=VAR_INT;
    }
    if (declstr) str=_str;
    str=spc(str);
    char *tstr=skipto(str,';',',');
    if (*(tstr-1)==',')
    {
        if (!declstr) declstr=_str;
    }
    else declstr=NULL;

    int i=tstr-str;
    t.Name=(char *)malloc(i);
    if (!t.Name) goto ErrorInGetDecl;
    strncpy(t.Name,str,i-1); t.Name[i-1]=0;

    return tstr;
ErrorInGetDecl:
    t.Type=VAR_UNKNOWN;
    if (t.Name) free(t.Name);
    return NULL;
}
int UDType::Assign(char *str)
{
    UDType::~UDType();
    char tmp[100];
    UDTypeEntry t;
    str=spc(str);
    // is it a 'struct' definition?
    if (next(str,"struct") || next(str,"union"))
    {
        Type=(next(str,"struct"))?VAR_STRUCT:VAR_UNION;

        str+=6;
        str=spc(str);
        // get the name field.
        next(str,"",tmp);
        Name=(char *)malloc(strlen(tmp)+1);
        if (!Name) goto ErrorInAssign;
        strcpy(Name,tmp);

        str=skipto(str,'{');
        str=spc(str);
        NumVars=0;
        char *tstr=str;
        while((tstr=getdecl(tstr,t))!=NULL)
        {
            NumVars++;
            tstr=spc(tstr);
            if (*tstr=='}') break;
        }
        Entries = (UDTypeEntry *)malloc(sizeof(UDTypeEntry)*NumVars);
        if (!Entries) goto ErrorInAssign;

        NumVars=0;
        while((str=getdecl(str,Entries[NumVars]))!=NULL)
        {
            NumVars++;
            str=spc(str);
            if (*str=='}') break;
        }
    }
    else if (next(str,"typedef"))
    {
        Type=VAR_TYPEDEF;
        str+=7; str=spc(str);

        Entries=(UDTypeEntry *)malloc(sizeof(UDTypeEntry));
        if (!Entries) goto ErrorInAssign;

        NumVars=1;
        Entries[0].ptr=NULL; Entries[0].Name=NULL;
        str=getdecl(str,Entries[0]);
    }
    return 0;
ErrorInAssign:
    UDType::~UDType();
    return 1;
}
UDTypeList::UDTypeList()
{
    Next=NULL;
}
UDTypeList::~UDTypeList()
{
    if (Next) delete Next;
    Next=NULL;
}
UDType *UDTypeList::Add(UDType &t)
{
    UDTypeList *ptr=this;
    while(ptr->Data.NumVars!=0)
    {
        if (!ptr->Next) ptr->Next = new UDTypeList;
        ptr=ptr->Next;
    }
    ptr->Data=t;
    return &ptr->Data;
}
UDType *UDTypeList::GetType(char *Name)
{
    UDTypeList *ptr=this;
    while(ptr && ptr->Data.NumVars!=0)
    {
        if (strcmp(ptr->Data.Name,Name)==0) return &ptr->Data;
        ptr=ptr->Next;
    }
    return NULL;
}
void UDTypeList::Display()
{
    UDTypeList *ptr=this;
    while(ptr && ptr->Data.NumVars!=0)
    {
        ptr->Data.Display();
        ptr=ptr->Next;
    }
}
/*----------------------------------------------------------
Routines which skip any comments while reading a C-file.
----------------------------------------------------------*/
int CurrentChar;
FILE *fileopen(char *fname,char *mode)
{
    FILE *fp=fopen(fname,mode);
    if (fp) CurrentChar=fgetc(fp);
    return fp;
}
void fileclose(FILE *fp)
{
    fileclose(fp);
}
int filegetc(FILE *fp)
{
    int NextChar;
    NextChar=fgetc(fp);
    if (CurrentChar=='/' && NextChar=='*')
    {
        do
        {
            CurrentChar=NextChar;
            NextChar=fgetc(fp);
        }while(!(CurrentChar=='*' && NextChar=='/'));
        CurrentChar=fgetc(fp); NextChar=fgetc(fp);
    }
    int c=CurrentChar; CurrentChar=NextChar;
    return c;
}
char *filegets(char *str,int n,FILE *fp)
{
    char *t=str;
    while(n--)
    {
        *str=filegetc(fp);
        if (*str==EOF || *str=='\n') break;
        str++;
    }
    *str=0;
    return t;
}
int fileread(char *str,int size,FILE *fp)
{
    int c,i;
    for(i=0;i<size && (c=filegetc(fp))!=EOF;i++) *(str++)=c;
    return i;
}
long fileseek(FILE *fp,int offset,int whence)
{
    fseek(fp,offset,whence);
    CurrentChar=fgetc(fp);
    return offset+1;
}
long filetell(FILE *fp)
{
    return ftell(fp)-1;
}
void FromFileToUDT(FILE *fp)
{
    long pos=filetell(fp);
    int c;
    int bracesleft=0;
    int size=0;
    do
    {
        c=filegetc(fp);
        size++;
        if (c==';' && !bracesleft) break;
        if (c=='{') bracesleft++;
        else if (c=='}') bracesleft--;
        if (bracesleft<0)
        {
            printf("Error in parsing datatype definition\n");
            break;
        }
    }while(1);
    char *str=(char *)malloc(size+1);
    if (!str)
    {
        printf("Not enough memory for new datatype definition\n");
        return;
    }
    fileseek(fp,pos,SEEK_SET);
    fileread(str,size,fp);
    str[size]=0;

    UDType t;
    t.Assign(str);
    udt.Add(t);
    t.Invalidate();
    free(str);
}
int GetTypesFromFile(char *FileName)
{
    FILE *fp;
    fp=fileopen(FileName,"r");
    if (!fp) return 1;
    char line[128];

    long fpos=0;
    while(!feof(fp))
    {
        filegets(line,127,fp);
        char *t;
        // Is it an #include statement?
        if (strncmp(line,"#include",8)==0)
        {
            t=skipto(line,'<','"');
            int len=skipto(t,'>','"')-t;
            t[len-1]=0;
            if (GetTypesFromFile(t)!=0)
	            printf("Error reading include file %s\n",t);
        }
        else if ((t=strstr(line,"typedef"))!=NULL ||
	         (t=strstr(line,"struct"))!=NULL ||
	         (t=strstr(line,"union"))!=NULL)
        {
            // seek to the beginning of whichever declaration we came across.
            fileseek(fp,fpos+t-line,SEEK_SET);
            // add this user-defined-type to the list.
            FromFileToUDT(fp);
        }
        fpos=filetell(fp);
    }
    return 0;
}
/*main()
{
  GetTypesFromFile("dos.h");
  udt.Display();
  return 0;
}
*/