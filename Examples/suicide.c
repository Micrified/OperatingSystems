#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int main (int argc, char *argv[]) {
  printf ("####\n");
  kill (getpid(), SIGKILL);
  printf ("Houston, we have a problem\n");
  return EXIT_SUCCESS;
}
