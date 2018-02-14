#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void handler (int sig) {
  printf ("\n\nhandler: caught signal %d\n", sig);
  if (sig == SIGINT) {
    printf ("See you next time\n");
    exit(EXIT_SUCCESS);
  }
  return;
}

int main (int argc, char *argv[]) {
  struct sigaction newsigact;
  sigset_t sset;

  newsigact.sa_handler = handler;
  sigemptyset(&newsigact.sa_mask);  /* signals to block while in handler */
  newsigact.sa_flags = 0;

  sigaction (SIGALRM, &newsigact, NULL);
  sigaction (SIGINT, &newsigact, NULL);

  alarm(1);  /* in 1 second a SIGALRM is send to this process */

  /* mask alarm signal */
  sigemptyset(&sset);
  sigaddset(&sset, SIGALRM);
  sigprocmask (SIG_BLOCK, &sset, NULL);
  

  while (1) {
    putchar ('#');
    fflush (stdout);
  }

  return EXIT_SUCCESS;
}
