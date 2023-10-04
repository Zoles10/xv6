#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// Fork and Print: Create a task where a parent process forks a child process. The child process should print a message, and the parent process should wait for the child to complete before printing its own message.

void forkAndPrint()
{
  int pid = fork();
  if (pid > 0)
  {
    wait(0);
    printf("Parent message!\n");
    ;
  }
  else if (pid == 0)
  {
    printf("Child message!\n");
  }
  else
  {
    exit(1);
  }
  exit(0);
}

// Inter-Process Communication: Develop a task where two processes communicate using a pipe. One process writes a message to the pipe, and the other process reads and prints the message. Ensure proper closing of file descriptors to avoid deadlocks.
int clearBuf(char buf[])
{
  for (int i = 0; i < strlen(buf); i++)
  {
    buf[i] = '\0';
  }
  exit(0);
}

int writeInBuf(char buf[], char word[])
{
  for (int i = 0; i < strlen(word); i++)
  {
    buf[i] = word[i];
  }
  exit(0);
}

void interProcessComms()
{
  int pipeFD[2];
  pipe(pipeFD);
  int pid = fork();
  if (pid == 0)
  {
    char buf[10] = {"\n\nHello "};
    write(pipeFD[1], buf, 10);
    close(pipeFD[1]);
    clearBuf(buf);
    read(pipeFD[0], buf, 10);
    printf("%s", buf);
    close(pipeFD[0]);
  }
  else if (pid > 0)
  {
    char buf[10];
    read(pipeFD[0], buf, 10);
    printf("%s\n", buf);
    clearBuf(buf);
    writeInBuf(buf, "World!");
    write(pipeFD[1], buf, 6);
  }
  else
  {
    exit(1);
  }
  exit(0);
}

// File Descriptor Duplication: Design a program that duplicates a file descriptor using the dup system call. For example, open a file, duplicate its descriptor, and use both descriptors to read and write simultaneously.

// Process Tree: Create a program that generates a hierarchical process tree using multiple fork calls. Each process should print its own PID and the PID of its parent process.

// Named Pipes (FIFOs): Explore the concept of named pipes (FIFOs) by creating two processes—one to write data into a FIFO and another to read from it. Ensure synchronization and proper cleanup.
// Pipeline Processing: Build a task where multiple processes form a pipeline. Each process in the pipeline reads data from its input, processes it in some way, and passes it to the next process through pipes. For example, you can implement a simple text transformation pipeline.

// File Redirection: Develop a program that demonstrates file redirection. Allow the user to specify input and output files as command-line arguments and use dup or similar calls to redirect input and output accordingly.

// Process Synchronization: Explore process synchronization concepts using fork. For instance, create a scenario where multiple child processes synchronize their execution using a shared resource (e.g., a semaphore or mutex).

// Signal Handling: Design tasks that involve signal handling using fork. One process can send a signal to another process, and the receiver should handle the signal appropriately (e.g., catching and ignoring it).

// Parallel Processing: Create a program that utilizes fork to parallelize a task, such as searching for a specific string in a large file. Measure and compare the execution time with and without parallelization.

int main(int argc, char *argv[])
{
  forkAndPrint();
  interProcessComms();
  exit(0);
}