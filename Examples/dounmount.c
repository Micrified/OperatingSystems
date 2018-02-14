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

  retval = umount ("/dev/c0d0p1s0");

  if (retval < 0) {
    fprintf (stderr, "unmount failed\n");
    return EXIT_FAILURE;
  }

  printf ("unmounted /dev/c0d0p1s0\n");


  return EXIT_SUCCESS;
}
