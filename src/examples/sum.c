#include <stdio.h>
#include <stdlib.h>
#include <syscall.h>

int
main (int argc, char **argv)
{
  int i;
  int a[4];

  if(argc!=5)
    exit(0);

  for (i = 0; i < 4; ++i)
    a[i] = atoi(argv[i+1]);
  printf("%d %d\n", fibonacci(a[0]), sum_of_four_integers(a[0],a[1],a[2],a[3]));
return 1023;
//  return EXIT_SUCCESS;
}
