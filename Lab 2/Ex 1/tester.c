#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>
#include <string.h>
#include <errno.h>

char msg[] = "Hello World";

int main (void) {
    int bytes, fd;

    if ((fd = open("helloWorld.txt", O_CREAT | O_WRONLY | O_TRUNC, 0755)) == -1) {
        fprintf(stderr, "Error: Couldn't create the file!\n");
        exit(-1);
    }

    if ((bytes = write(fd, msg, sizeof(msg))) != sizeof(msg)) {
        fprintf(stderr, "Error: Was supposed to write %lu bytes but actually wrote %d!\n", sizeof(msg), bytes);
        exit(-1);
    }
    close(fd);
    return 0;
}

        // Link standard I/O.
        replaceIn(fd_in);
        replaceOut(fd_out);

        // Link explicit I/O.
        if (p->in != NULL ) {
            replaceIn(openInFile(p->in));
        }
        if (p->out != NULL) {
            replaceOut(openOutFile(p->out));
        }

        // Display Error if bad program.
        if (execve(p->name, p->args, NULL) == -1) {
            fprintf(stderr, "Error: Couldn't run \"%s\"!\n", p->name);
            exit(-1);
        }