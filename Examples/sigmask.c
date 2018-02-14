#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int main (int argc, char *argv[]) {
  struct sigaction newsigact;
  sigset_t sset;


  /* mask alarm signal */
  sigemptyset(&sset);
  sigaddset(&sset, SIGALRM);
  sigprocmask (SIG_BLOCK, &sset, NULL);
  
  alarm(1);  /* in 1 second a SIGALRM is send to this process */

  while (1) {
    putchar ('#');
    fflush (stdout);
  }

  return EXIT_SUCCESS;
}
