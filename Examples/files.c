#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

void error (char *msg) {
  fprintf (stderr, "%s", msg);
  printf ("errno=%d\n", errno);
  exit(EXIT_FAILURE);
}

int main (int argc, char *argv[]) {
  int fd, n;
  char msg[32] = "Hello World!";

  
  fd = creat("demo.txt", 0755);
  if (fd < 0) {
    error ("creat failed\n");
  }
  write (fd, msg, 1+strlen(msg));
  close(fd);
  
  fd = open("demo.txt", O_RDONLY);
  if (fd < 0) {
    error ("open (O_RDONLY) failed\n");
  }
  n = read (fd, msg, 32);
  close(fd);
  
  printf ("%d: %s\n", n, msg);

  fd = open ("demo2.txt", O_WRONLY | O_CREAT | O_TRUNC, 0755);
  if (fd < 0) {
    error ("open (O_WRONLY | O_CREAT | O_TRUNC, 0755) failed\n");
  }
  write (fd, msg, 1+strlen(msg));
  close(fd);

  fd = open("demo2.txt", O_RDONLY);
  if (fd < 0) {
    error ("open (O_RDONLY) failed\n");
  }
  n = read (fd, msg, 32);
  close(fd);
  printf ("%d: %s\n", n, msg);

  return EXIT_SUCCESS;
}
