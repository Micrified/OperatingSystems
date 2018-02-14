#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int main (int argc, char *argv[]) {
  int child;

  if ((child=fork()) != 0) {
    /* Parent process gets here */
    sleep(1);
    kill (child, SIGKILL);
  } else {
    while (1) {
      putchar ('#');
      fflush(stdout);
    }
  }

  return EXIT_SUCCESS;
}
