#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
  int fds[2];
  pipe(fds);
  int pid = fork();

  if (pid == 0)
  {
    char buf[1];
    read(fds[0], buf, 1);
    close(fds[0]);
    printf("%d: received ping\n", getpid());
    write(fds[1], "a", 1);
    close(fds[1]);
  }
  else if (pid > 0)
  {
    write(fds[1], "a", 1);
    close(fds[1]);
    char buf[1];
    read(fds[0], buf, 1);
    close(fds[0]);
    printf("%d: received pong\n", getpid());
  }
  else
  {
    exit(1);
  }
  exit(0);
}
