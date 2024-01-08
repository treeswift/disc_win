#include <stdio.h>
main()
{
  int i=4, j;
  printf("abc %p,%d",&i,(i=4,8));
  for(j=0; j<10; j++)
    i=j*i+i;
}
