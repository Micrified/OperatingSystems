#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/file.h>
#include <assert.h>
#include <errno.h>

/*
*******************************************************************************
*                            stdin/stdout Routines                            *
*******************************************************************************
*/

// Aliases for the program's stdin/stdout file-descriptors.
int alias_stdin = 0, alias_stdout = 1;

/* Replaces stdin with the given file-descriptor. Returns duplicate fd. */
int replaceIn (int fd) {
    alias_stdin = dup(alias_stdin);
    close(STDIN_FILENO);
    return dup(fd); // Replace stdin, return new file-descriptor.
}

/* Restores stdin */
void restoreIn () {
    close(STDIN_FILENO);
    alias_stdin = dup(alias_stdin);
}

/* Replaces stdout with the given file-descriptor. Returns duplicate fd */
int replaceOut (int fd) {
    alias_stdout = dup(alias_stdout);
    close(STDOUT_FILENO);
    return dup(fd); // Replace stdout, return new file-descriptor.
}

/* Restores stdout */
void restoreOut () {
    close(STDOUT_FILENO);
    alias_stdout = dup(alias_stdout);
}

/*
*******************************************************************************
*                               Pipe Routines                                 *
*******************************************************************************
*/

/* Forks master. Child replaces stdin and stdout with given file-descriptors.
 * Child then executes the given program. Master returns the childs PID.
*/
int execProgram (int fd_in, int fd_out, char *p, char **pargs) {
    int pid;
    if ((pid = fork()) == 0) { // Todo: If child has indirection, replace that here.
        replaceIn(fd_in);
        replaceOut(fd_out);
        execve(p, pargs, NULL);
    }
    return pid;
}

/* Initializes a pipe, then forks. Child replaces stdin with fd_in, and 
 * configures itself to write to the pipe's writing file-descriptor. 
 * The child then executes the given program. Meanwhile, master returns
 * the reading file-descriptor. 
*/
int setPipe (int fd_in, char *p, char **pargs) {
    int fds[2];
    assert((pipe(fds) != -1) && "Error: Pipe creation failed!\n");

    if (fork() == 0) {
        execProgram(fd_in, fds[1], p, pargs);
    }
    return fds[0];
}

/* Initializes and launches a sequence of programs interlinked by pipes. 
 * Unless otherwise overwritten, the first program of the sequence reads 
 * from stdin. Likewise, the final program writes to stdout. The last
 * programs PID is then returned.
*/
int setPipeSequence (void) {
    int fd_in = alias_stdin;
    prgm *p = nextProgram(), *next = nextProgram();

    while (next != NULL) {
        fd_in = setPipe(fd_in, p->name, p->args);
        p = next;
        next = nextProgram();
    }

    return execProgram(fd_in, alias_stdout, p->name, p->args);
}

int runCommandSequence (void) {
    while ((ps = nextPipeSequence()) != NULL) {
        if (accept(AMP)) {
            if (fork() == 0) {
                waitpid(setPipeSequence(ps));
            }
            continue;
        }
        
        // Wait if semicolon or not.
        waitpid(setPipeSequence(ps));
    }
}


int main (int argc, const char *argv[]) {
    if (argc != 3) {
        printf("Main: <in> <out>\n");
        exit(-1);
    }
    printf("Main: cat < %s > %s\n", argv[1], argv[2]);

    runCatWithFile(argv[1], argv[2]);

    printf("\nMain: Write a number to finish...\n");
    int n = 0;
    scanf("%d", &n);
    printf("Main: Thanks, got %d\n", n);
    return EXIT_SUCCESS;
}