#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

  
int main (int argc, char *argv[]) {
  int retval;

  if (geteuid() != 0) {
    fprintf (stderr, "Sorry, only root can do this!\n");
    exit (EXIT_FAILURE);
  }

  /* 0=read/write, 1=read-only */
  retval = mount ("/dev/c0d0p1s0", "/home/arnold/examples/mnt", 0);

  if (retval < 0) {
    fprintf (stderr, "mount failed\n");
    return EXIT_FAILURE;
  }

  printf ("mounted /dev/c0d0p1s0 on /home/arnold/examples/mnt\n");


  return EXIT_SUCCESS;
}
