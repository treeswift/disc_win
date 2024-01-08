#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef unsigned char Byte;
Byte *Table;
int TableSize;
FILE *fp;
void WriteFunctions()
{
  fprintf(fp,"Functions\n");
  Byte *SymTable = Table + 0x30 + *(short int *)(Table+0x2E);
  int NumGlobalSymbols = *(short int *)(Table+0x10);
  int SizeOfNamePool   = *(long int *)(Table+0x04);
  Byte *Names = Table + (TableSize - SizeOfNamePool);
  for(int i=0;i<NumGlobalSymbols;i++)
  {
    int Type = *(short int *)(SymTable+0x02);
    unsigned int Offset = *(unsigned short int *)(SymTable+0x04);
    unsigned int Segment= *(unsigned short int *)(SymTable+0x06);
    if (Type>18)
    {
      switch(Type)
      {
	case 0x1A : fprintf(fp,"%X:%X int %s\n",Segment,Offset,Names); break;
	case 0x1B : fprintf(fp,"%X:%X char %s\n",Segment,Offset,Names); break;
	case 0x1C : fprintf(fp,"%X:%X void %s\n",Segment,Offset,Names); break;
	default   : fprintf(fp,"%X:%X unknown %s\n",Segment,Offset,Names); break;
      }
    }
    SymTable += 9;
    Names += strlen(Names)+1;
  }
}
void WriteGlobalVars()
{
  fprintf(fp,"Globals\n");
  Byte *SymTable = Table + 0x30 + *(short int *)(Table+0x2E);
  int NumGlobalSymbols = *(short int *)(Table+0x10);
  int SizeOfNamePool   = *(long int *)(Table+0x04);
  Byte *Names = Table + (TableSize - SizeOfNamePool);
  for(int i=0;i<NumGlobalSymbols;i++)
  {
    int Type = *(short int *)(SymTable+0x02);
    unsigned int Offset = *(unsigned short int *)(SymTable+0x04);
    unsigned int Segment= *(unsigned short int *)(SymTable+0x06);
    switch(Type)
    {
      case 0x01 : fprintf(fp,"%X:%X void %s\n",Segment,Offset,Names); break;
      case 0x02 : fprintf(fp,"%X:%X char %s\n",Segment,Offset,Names); break;
      case 0x04 : fprintf(fp,"%X:%X int %s\n",Segment,Offset,Names); break;
      case 0x06 : fprintf(fp,"%X:%X long %s\n",Segment,Offset,Names); break;
      case 0x18 : fprintf(fp,"%X:%X struct %s\n",Segment,Offset,Names); break;
      default   : if (Type<=0x18)
	fprintf(fp,"%X:%X unknown %s\n",Segment,Offset,Names); break;
    }
    SymTable += 9;
    Names += strlen(Names)+1;
  }
}
void WriteLocalVars()
{
  fprintf(fp,"Locals\n");
}
void ReadSymbolTable()
{
  if (Table) free(Table);
  Table=NULL;
  fseek(fp,0,SEEK_END);
  long FileSize=ftell(fp);
  fseek(fp,0,SEEK_SET);
  long BytesRead;
  Byte data[16];
  do
  {
    BytesRead=fread(data,1,16,fp);
    if (data[0]==0xFB && data[1]==0x52) break;
  }while(BytesRead==16);
  if (BytesRead!=16) { puts("No symbolic info in file."); return; }
  TableSize = FileSize - ftell(fp) + 16;
  Table = (Byte *)malloc(TableSize);
  if (!Table) { puts("Not enough memory."); return; }
  fseek(fp,-16,SEEK_CUR);
  fread(Table,TableSize,1,fp);
}
main(int argc,char *argv[])
{
  if (argc==1) return 0;
  fp = fopen(argv[1],"rb");
  if (!fp) { puts("Unable to open file."); return 1; }
  ReadSymbolTable();
  if (!Table) return 1;
  fclose(fp);

  char filename[40];
  strcpy(filename,argv[1]);
  int i;
  for(i=0;filename[i+1]!=0;i++)
    if (filename[i]=='.') break;
  strcpy(&filename[i+1],"STB");

  fp=fopen(filename,"w");
  if (!fp) { puts("Unable to create output file."); return 1; }
  WriteFunctions();
  WriteGlobalVars();
  WriteLocalVars();
  fclose(fp);
  return 0;
}