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

#define CHUNK 16

int main (int argc, char *argv[]) {
  int fd, len, pos;
  char msg[CHUNK];

  fd = open("demo.txt", O_RDONLY);
  if (fd < 0) error ("failed to open file 'demo.txt'\n");

  /* get length of file */
  len = lseek(fd, 0, SEEK_END);
  printf ("len=%d\n", len);

  for (pos = 0; pos < len - CHUNK + 1; pos++) {
    lseek(fd, pos, SEEK_SET);
    read(fd, msg, CHUNK-1);
    msg[CHUNK-1] = '\0';
    printf ("%s\n", msg);
  }
  printf("###############\n");
  for (pos=CHUNK; pos <= len; pos++) {
    lseek(fd, -pos, SEEK_END);
    read(fd, msg, CHUNK-1);
    msg[CHUNK-1] = '\0';
    printf ("%s\n", msg);
  }

  close(fd);

  return EXIT_SUCCESS;
}
