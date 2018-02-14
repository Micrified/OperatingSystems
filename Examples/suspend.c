#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void handler (int sig) {
  printf ("handler: caught signal %d\n", sig);
  return;
}

int main (int argc, char *argv[]) {
  int s;
  struct sigaction newsigact;

  newsigact.sa_handler = handler;
  sigemptyset(&newsigact.sa_mask);  /* signals to block while in handler */
  newsigact.sa_flags = 0;

  sigaction (SIGALRM, &newsigact, NULL);
  alarm(1);  /* this will terminate the program in 1 sec. */

  printf ("#####");
  fflush(stdout);
  
  s = pause();

  printf ("s=%d\n", s);
  return EXIT_SUCCESS;
}
