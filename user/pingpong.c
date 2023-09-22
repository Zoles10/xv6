#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main()
{
  int fds[2];
  pipe(fds);
  int process = fork();
  if (process > 0)
  {
    char *buff[1];
    buff[0] = "a";
    write(fds[1], buff, 1);
    close(fds[1]);
    read(fds[0], buff, 1);
    fprintf(0, "<%d>: received pong\n", getpid());
    close(fds[0]);
  }
  else if (process == 0)
  {
    char *buff[1];
    read(fds[0], buff, 1);
    fprintf(0, "<%d>: received ping\n", getpid());
    write(fds[1], buff, 1);
    close(fds[0]);
    close(fds[1]);
  }
  else if (process < 0)
  {
    exit(1);
  }
  exit(0);
}
