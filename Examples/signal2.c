#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void handler (int sig) {
  static int cnt;
  printf ("handler: caught signal %d\n", sig);
  if (cnt++ == 2) exit(EXIT_SUCCESS);
  alarm(1);  /* in 1 second a SIGALRM is send to this process */
  return;
}

int main (int argc, char *argv[]) {
  struct sigaction newsigact;
  struct sigaction oldsigact;

  newsigact.sa_handler = handler;
  sigemptyset(&newsigact.sa_mask);  /* signals to block while in handler */
  newsigact.sa_flags = 0;

  sigaction (SIGALRM, &newsigact, &oldsigact);

  alarm(1);  /* in 1 second a SIGALRM is set to this process */

  while (1) {
    putchar ('#');
    fflush (stdout);
    usleep(1000);
  }

  return EXIT_SUCCESS;
}
