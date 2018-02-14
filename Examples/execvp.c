#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


int main (int argc, char *argv[]) {
  char *newargv[] = {"ls", "-l", NULL};

  printf ("The output of 'ls -l' is:\n");
  execvp ("ls", newargv);

  /* we will never get here */
  printf ("Houston we have a problem\n");

  return EXIT_SUCCESS;
}
