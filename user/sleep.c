#include "kernel/sysproc.c"

void sleep(int n)
{
  sys_sleep(n);
}
void main(int argc, char *argv[]){
  sleep(atoi(*argv[1]));
  exit(0);
}

