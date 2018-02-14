#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int main (int argc, char *argv[]) {
  alarm(1);  /* this will terminate the program in 1 sec. */

  while (1) {
    putchar ('#');
    fflush(stdout);
  }

  return EXIT_SUCCESS;
}
