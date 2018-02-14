#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define STDIN  0
#define STDOUT 1
#define STDERR 2

int main (int argc, char *argv[]) {
  char msg[256] = "Hello world!\n";
  int newfd;

  newfd = dup(STDOUT);

  write (STDOUT, msg, 1+strlen(msg));
  write (newfd, msg, 1+strlen(msg));

  return EXIT_SUCCESS;
}
