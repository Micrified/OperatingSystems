#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main (int argc, char *argv[]) {
  int parent, child, status;

  parent = getpid();

  child = fork();
  if (child < 0) {
    fprintf (stderr, "Fork failed: abort\n");
    return EXIT_FAILURE;
  }

  if (child != 0) {
    /* Parent process gets here */
    printf ("parent: Mypid=%d, child=%d\n", parent, child);
  } else {
    *((char *)42) = 42;
    exit(99);
  }
  
  waitpid(-1, &status, 0);
  printf ("status=%d,%d\n", status/256, status%256);

  return EXIT_SUCCESS;
}
