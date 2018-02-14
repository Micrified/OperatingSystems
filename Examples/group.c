#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main (int argc, char *argv[]) {
  printf ("getpid:  %d\n", getpid());
  printf ("getpgrp: %d\n", getpgrp());

  setsid();
  printf ("getpgrp: %d\n", getpgrp());
  return EXIT_SUCCESS;
}
