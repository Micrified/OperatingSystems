#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define STDIN  0
#define STDOUT 1
#define STDERR 2

int main (int argc, char *argv[]) {
  char msg[256];
  int n;

  printf ("Please type something: ");
  fflush(stdout);
  n = read (STDOUT, msg, 256);
  printf ("You type %d characters: %s", n, msg);

  sprintf (msg, "This goes to stdout\n");
  write (STDOUT, msg, strlen(msg)+1);

  sprintf (msg, "This goes to stderr\n");
  write (STDERR, msg, strlen(msg)+1);

  return EXIT_SUCCESS;
}
