#include <dos.h>
#include <dir.h>
#include <stdio.h>
main(int argc, char *argv[])
{
  struct ffblk ffblk;
  long size, i;
  unsigned short CodeSegment;
  union REGS regs;
  struct SREGS sregs;
  unsigned short ParamBlock[2];
  FILE *fp;

  if (argc < 2)
  {
    printf("This program should be used within DISC!");
    exit(0);
  }
  if (findfirst(argv[1], &ffblk, 0x17) == -1)
    exit(1);
  size = ffblk.ff_fsize;
  if (allocmem((size/16)+1, (unsigned *)&CodeSegment) != -1)
    exit(2);
  ParamBlock[0] = CodeSegment;
  ParamBlock[1] = CodeSegment;

  regs.x.ax = 0x4b03;
  sregs.ds  = FP_SEG((void far *)argv[1]);
  regs.x.dx = FP_OFF((void far *)argv[1]);
  sregs.es  = FP_SEG((void far *)ParamBlock);
  regs.x.bx = FP_OFF((void far *)ParamBlock);
  int86x(0x21,&regs,&regs,&sregs);
  if (regs.x.cflag == 1)
    exit(3);

  fp = fopen("disc_mem.dmp", "wb");
  if (fp == NULL)
    exit(4);
  fwrite(&CodeSegment, 2, 1, fp);
  for(i=0; i<size; i++)
  {
    unsigned char c;
    unsigned char far *ptr = (unsigned char far *)MK_FP(CodeSegment, 0);
    c = *(ptr + i);
    fwrite(&c, 1, 1, fp);
  }
  fclose(fp);
  freemem(CodeSegment);
  exit(0);
}