#include <stdio.h>
#include <process.h>
main()
{
  int i=4,j,k;
  char c;
  exit(0);
  scanf("%d",&j);
  if (i<j)
  {
    printf("i<j");
    i=j;
    c=4;
  }
  else if (i>j)
  {
    printf("i>j");
    j=4;
    k=2;
    switch((i<=j)?j:k)
    {
      case 0 : if (i<j)
		{
		  printf("i<j");
		  i=j;
		}
		else if (i>j && c==1)
		{
		  printf("i>j");
		  j=4;
		}
		break;
      case 22 : printf("22"); break;
      case 19 : printf("a");
      case -4 : i=1; break;
      default : for(i=0;i<10;i++) i=i+i*i;
    }
  }
  else
  {
    printf("i==j");
    k=1;
  }
}
