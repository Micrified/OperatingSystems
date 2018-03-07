#include "eval.h"

// Minimum program argument count (1 for program name)
#define PROGRAM_MIN_ARGC    1

// An invalid file-descriptor constant.
#define FD_NONE             -1

// Max.
#define MAX(a,b)            ((a) > (b) ? (a) : (b))

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
    safeClose(STDIN_FILENO);
    return safeDup2(fd, STDIN_FILENO); // Replace stdin, return new descriptor.
}

/* Replaces stdout with the given file-descriptor. Returns duplicate fd */
static int replaceOut (int fd) {
    if (fd == STDOUT_FILENO) return fd;
    safeClose(STDOUT_FILENO);
    return safeDup2(fd, STDOUT_FILENO); // Replace stdout, return new descriptor.
}

/*
*******************************************************************************
*                             Auxiliary Routines                              *
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

/*
*******************************************************************************
*                          Program Parsing Routines                           *
*******************************************************************************
*/

/* Allocates and initializes a Program instance with given attributes */
static Program *newProgram (char *name, char **args, char *in, char *out) {
    Program *p = malloc(sizeof(Program));
    assert(p != NULL);
    *p = (Program){.name = name, .args = args, .in = in, .out = out};
    return p;
}

/* Frees a Program instance */
static void freeProgram (Program *p) {
    free(p->args);
    free(p);
}

/* Parses a program from the queue and returns an initialized "Program" type.
 * If no program exists, NULL is returned. */
static Program *parseProgram () {
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

/* Applies piping and redirection to fork. Tries to exec on all paths */
static int execProgram (int fd_in, int fd_out, Program *p) {
        char *path, *buffer = NULL;
        int bufferSize = 0;

        // Apply implicit redirection.
        replaceIn(fd_in);
        replaceOut(fd_out);

        // Apply explicit redirection.
        if (p->in != NULL) {
            replaceIn(safeOpen(p->in, O_RDONLY));
        }
        if (p->out != NULL) {
            replaceOut(safeOpenMode(p->out, O_CREAT | O_WRONLY | O_TRUNC, 0755));
        }

        // Attempt to execute local program.
        execv(p->name, p->args);

        // Attempt to execute on all paths.
        while ((path = nextPath()) != NULL) {
            bufferSize = MAX(bufferSize, strlen(path) + strlen(p->name) + 2);
            buffer = resizeBuffer(buffer, bufferSize, sizeof(char));
            sprintf(buffer, "%s/%s", path, p->name);
            execv(buffer, p->args);
        }
        free(buffer);
        return -1;
}

/* Forks shell. Free's program and returns if parent. Executes program if child */
static int evalProgram (int fd_in, int fd_out, int fd_close, Program *p) {
    int pid;

    // Return child PID if parent.
    if ((pid = fork()) != 0) {
        freeProgram(p);
        return pid;
    }

    // Search for program on all paths. Print error if failure.
    if (fd_close != FD_NONE) safeClose(fd_close);
    execProgram(fd_in, fd_out, p);
    fprintf(stderr, "Error: Couldn't execute \"%s\"!\n", p->name);    
    freeProgram(p);
    exit(EXIT_FAILURE);
}

/* Initializes a pipe. Lauches program with given fd_in as stdin. Hooks stdout to pipe.
 * Then returns other reading file-descriptor back so it can be hooked to next pipe 
 * [Important]: Sends reading descriptor to be closed by fork. And closes writing
 *  so that no two forks will have the same file-descriptors open. */
static int setPipe (int fd_in, Program *p) {
    int fds[2];
    safePipe(fds);
    evalProgram(fd_in, fds[1], fds[0], p); // Pass read port so fork can close it for us.
    safeClose(fds[1]); // Close Writing descriptor to hand control to fork.
    return fds[0];
}

/* Links and actively launches programs in a pipe. If only one program, it reads
 * from stdin and outputs to stdout. If several programs are piped, the first 
 * program reads from stdin, and writes to the writing-fd of the pipe. The 
 * reading-fd gets returned and given as the next stdin of the following program.
 * the last program is set to write to stdout unless explicity redirected */
static int evalPipeSequence() {
    int fd_in = STDIN_FILENO;
    Program *p = parseProgram(), *next = NULL;

    while (!IS_NULL(getHead()) && getHead().type == PIPE) {
        dequeue();
        next = parseProgram();
        fd_in = setPipe(fd_in, p);
        p = next;
    }

    return evalProgram (fd_in, STDOUT_FILENO, FD_NONE, p);
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

    // If nothing is in the queue, don't process it.
    if (queueSize() <= 0) {
        return;
    }

    // Process the pipe-sequence. Obtain PID of last program.
    pid = evalPipeSequence();

    // Dequeue possible separator. If "&" then fork shell. Otherwise wait.
    if (!IS_NULL((item = dequeue())) && item.type == AMPERSAND) {
        if (fork() == 0) {
            waitpid(pid, &status, 0);
            exit(0);
        }
    } else {
        waitpid(pid, &status, 0);
    }

    // Print prompt token.
    printf("iwish$ ");
}