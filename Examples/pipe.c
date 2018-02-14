#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main (int argc, char *argv[]) {
  int fd[2], status;
  char msg[256];

  if (pipe(fd) < 0) {
    fprintf (stderr, "Could not make pipe\n");
    return (EXIT_FAILURE);
  }

  if (fork() != 0) {
    /* Parent process gets here */
    close(fd[0]);  /* fd[0] is used for reading */
    sprintf (msg, "Hello world!\n");
    write (fd[1], msg, 256); /* fd[1] is used for writing */
  } else {
    /* Child process gets here */
    close(fd[1]);
    read (fd[0], msg, 256);
    printf ("%s", msg);
  }

  waitpid(-1, &status, 0);

  return EXIT_SUCCESS;
}
