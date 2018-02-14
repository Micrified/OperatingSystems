#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

void error (char *msg) {
  fprintf (stderr, "%s", msg);
  printf ("errno=%d\n", errno);
  exit(EXIT_FAILURE);
}

int main (int argc, char *argv[]) {
  struct stat filestatus;
  int retval;
  
  retval = stat ("demo.txt", &filestatus);
  if (retval < 0) {
    error ("stat failed\n");
  }

  printf ("st_dev   = %d\n", (int)filestatus.st_dev);
  printf ("st_ino   = %d\n", (int)filestatus.st_ino);
  printf ("st_mode  = %d\n", (int)filestatus.st_mode);
  printf ("st_nlink = %d\n", (int)filestatus.st_nlink);
  printf ("st_uid   = %d\n", (int)filestatus.st_uid);
  printf ("st_gid   = %d\n", (int)filestatus.st_gid);
  printf ("st_size  = %d\n", (int)filestatus.st_size);
  printf ("st_atime = %d\n", (int)filestatus.st_atime);
  printf ("st_mtime = %d\n", (int)filestatus.st_mtime);

  return EXIT_SUCCESS;
}
