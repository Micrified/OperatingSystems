#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include "queue.h"
#include "iwish.tab.h"
#include "strtab.h"

#define PROGRAM_MIN_ARGC    1

/*
*******************************************************************************
*                                 Data Types                                  *
*******************************************************************************
*/

/* Describes program parameters required for execution */
typedef struct {
    char *name, **args, *in, *out;
} Program;

/*
*******************************************************************************
*                            stdin/stdout Routines                            *
*******************************************************************************
*/

/* Replaces stdin with the given file-descriptor. Returns duplicate fd. */
static int replaceIn (int fd) {
    if (fd == STDIN_FILENO) return fd;
    close(STDIN_FILENO);
    return dup2(fd, STDIN_FILENO); // Replace stdin, return new file-descriptor.
}

/* Replaces stdout with the given file-descriptor. Returns duplicate fd */
static int replaceOut (int fd) {
    if (fd == STDOUT_FILENO) return fd;
    close(STDOUT_FILENO);
    return dup2(fd, STDOUT_FILENO); // Replace stdout, return new file-descriptor.
}

/*
*******************************************************************************
*                             Utility Routines                                *
*******************************************************************************
*/

/* Resizes a buffer to given size, or allocates it if NULL. Returns pointer */
static void *resizeBuffer (void *buffer, unsigned n, unsigned typeSize) {
    if (buffer == NULL) {
        buffer = malloc(n * typeSize);
    } else {
        buffer = realloc(buffer, n * typeSize);
    }
    assert(buffer != NULL && "Error: Failure when resizing buffer!\n");
    return buffer;
}

/* Attempts to open a file for reading. Aborts on failure */
int openInFile (const char *fileName) {
    int fd;
    if ((fd = open(fileName, O_RDONLY)) == -1) {
        fprintf(stderr, "Error: Can't open \"%s\" for reading!\n", fileName);
        exit(-1);
    }
    return fd;
}

/* Attempts to open or create a file for writing. Aborts on failure */
int openOutFile (const char *fileName) {
    int fd;
    if ((fd = open(fileName, O_CREAT | O_WRONLY | O_TRUNC, 0755)) == -1) {
        fprintf(stderr, "Error: Can't create or write to \"%s\"!\n", fileName);
        exit(-1);
    }
    return fd;
}

/*
*******************************************************************************
*                          Program Parsing Routines                           *
*******************************************************************************
*/

/* Allocates and initializes a Program instance with given attributes */
Program *newProgram (char *name, char **args, char *in, char *out) {
    Program *p = malloc(sizeof(Program));
    assert(p != NULL);
    *p = (Program){.name = name, .args = args, .in = in, .out = out};
    return p;
}

/* [DEBUG] Prints a Program instance */
void printProgram (Program *p) {
    if (p == NULL) {
        printf("{ NULL }\n");
        return;
    }

    printf("{ Program: \"%s\", ", p->name);
    printf("In = %s, ", (p->in == NULL ? "None" : p->in));
    printf("Out = %s, ", (p->out == NULL ? "None" : p->out));
    printf("Args = [");
    char **args = p->args;
    while (*args != NULL) {
        printf("\"%s\"", *args);
        if (*(args + 1) != NULL) {
            printf(",");
        }
        args++;
    }
    printf("] }\n");
}

/* Parses a program from the queue and returns an initialized "Program" type.
 * If no program exists, NULL is returned. */
Program *parseProgram () {
    unsigned i = 0, argc = PROGRAM_MIN_ARGC;
    Item arg;
    char *name = NULL, **args = NULL, *in = NULL, *out = NULL;

    // If no program in queue. Return NULL.
    if (IS_NULL(getHead()) || (!IS_NULL(getHead()) && getHead().type == PIPE)) {
        return NULL;
    }

    // Parse: name.
    name = getString(dequeue().widx);

    // Initialize args buffer.
    args = (char **)resizeBuffer(args, argc, sizeof(char *));

    // Set first argument to program name.
    args[i++] = name;

    // Parse: args + redirections.
    while (!IS_NULL((arg = getHead())) && 
            ((arg.type == WORD) || (arg.type == IN) || (arg.type == OUT))) {
        dequeue();
        if (arg.type == IN) {
            in = getString(dequeue().widx);
            continue;
        }
        if (arg.type == OUT) {
            out = getString(dequeue().widx);
            continue;
        }

        // Resize args such that one extra space always reserved.
        if (i >= argc) {
            args = (char **)resizeBuffer(args, ++argc, sizeof(char *));
        }
        args[i++] = getString(arg.widx);
    }

    // Null-terminate args (execve requirement).
    args = (char **)resizeBuffer(args, ++argc, sizeof(char *));
    args[i] = NULL;
    return newProgram(name, args, in, out);
}

/*
*******************************************************************************
*                           Pipe Parsing Routines                             *
*******************************************************************************
*/

int evalProgram (int fd_in, int fd_out, Program *p) {
    int pid;
    printf("Launching -> "); printProgram(p);
    if ((pid = fork()) == 0) {
        
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
            fprintf(stderr, "More: %s\n", strerror(errno));
            exit(-1);
        }
        exit(0);
    }
    return pid;
}


int setPipe (int fd_in, Program *p) {
    int fds[2];
    assert((pipe(fds) != -1) && "Error: Pipe creation failed!\n");
    evalProgram(fd_in, fds[1], p);
    return fds[0];
}

static int evalPipeSequence() {
    int fd_in = STDIN_FILENO;
    Program *p = parseProgram(), *next = NULL;

    while (!IS_NULL(getHead()) && getHead().type == PIPE) {
        dequeue();
        next = parseProgram();
        fd_in = setPipe(fd_in, p);
        free(p); p = next;
    }

    return evalProgram (fd_in, STDOUT_FILENO, p);
}

/*
*******************************************************************************
*                                Eval Routines                                *
*******************************************************************************
*/

/* Evaluates all items on the queue (I.E. command execution) */
void evalQueue () {
    int pid, status;
    Item item;
    printf("-------------------- Evaluating --------------------\n");
    // Process the pipe-sequence. Obtain PID of last program.
    pid = evalPipeSequence();
    printf("\nLast PID = %d...\n\n", pid);
    // Dequeue potential separator. 
    // If separator exists and is "&" -> fork the shell.
    //  - If child of fork -> wait on PID.
    //  - If parent -> return.
    // Else separator not exists or is ";" -> wait on PID.
    if (!IS_NULL((item = dequeue())) && item.type == AMPERSAND) {
        printf("Done: PID = %d, MODE = ASYNC\n", pid);
        if (fork() == 0) {
            waitpid(pid, &status, 0);
        }
    } else {
        printf("Done: PID = %d, MODE = SERIAL\n", pid);
        waitpid(pid, &status, 0);
    }
    printf("Check: Queue Empty ? %s\n", queueSize() == 0 ? "Yes" : "No");
    printf("-------------------- ---------- --------------------\n");
}