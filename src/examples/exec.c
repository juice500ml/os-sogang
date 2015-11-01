#include <stdio.h>
#include <syscall.h>

int main(void)
{
  wait (exec ("child-simple"));
  return 321;
}
