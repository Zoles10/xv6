#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
  if (argc <= 1)
  {
    fprintf(2, "Enter a number");
    exit(1);
  }

  if (argc > 2)
  {
    fprintf(2, "Too many arguments\n");
    exit(1);
  }

  int seconds = atoi(argv[1]);

  sleep(seconds);

  exit(0);
}
