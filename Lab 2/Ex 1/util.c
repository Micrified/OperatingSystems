#include "util.h"

/*
*******************************************************************************
*                             Global Variables                                *
*******************************************************************************
*/

/* Pointer to the environment paths string */
char *envp;

/* Number of paths in the environment-path */
int pathCount = 0;

/*
*******************************************************************************
*                          Internal Utility Routines                          *
*******************************************************************************
*/

/* Returns the number of times 'c' occurs in 's' */
static int numchar (char c, const char *s) {
    int n = 0;
    while (*s != '\0') {
        n += (*(s++) == c);
    }
    return n;
}

/* Initializes the environment path. 
 * - Counts the number of paths and sets pathCount.
 * - Allocates a copy to envp, and replaces colons with null-char.
*/
static void initEnvPaths () {
    const char *paths = getenv("PATH");

    // Compute and set number of paths.
    if (strlen(paths) == 0) {
        pathCount = 0;
    } else {
        pathCount = numchar(':', paths) + 1;
    }

    // Copy paths to envp.
    envp = malloc((strlen(paths) + 1) * sizeof(char));
    assert(envp != NULL);
    strcpy(envp, paths);

    // Replace colons with null-char;
    char *p = envp;
    while (*p != '\0') {
        if (*p == ':') {
            *p = '\0';
        }
        p++;
    }
}

/*
*******************************************************************************
*                           Syscall Utility Routines                          *
*******************************************************************************
*/

/* Closes file-descriptor. Aborts with message on error */
void safeClose (int fd) {
    if (close(fd) == -1) {
        fprintf(stderr, "Error (close): \"%s\"\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

/* Opens file with flags. Aborts with message on error */
int safeOpen (const char *fileName, int flags) {
    int fd;
    if ((fd = open(fileName, flags)) == -1) {
        fprintf(stderr, "Error (open): \"%s\"\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return fd;
}

/* Opens file with flags and mode. Aborts with message on error */
int safeOpenMode (const char *fileName, int flags, mode_t mode) {
    int fd;
    if ((fd = open(fileName, flags, mode)) == -1) {
        fprintf(stderr, "Error (openMode): \"%s\"\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return fd;
}

/* Opens pipe with given buffer. Aborts with message on error */
void safePipe(int pipefds[2]) {
    if (pipe(pipefds) == -1) {
        fprintf(stderr, "Error (pipe): \"%s\"\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

/* Copies a file descriptor. Aborts with message on error */
int safeDup2 (int oldfd, int newfd) {
    int fd;
    if ((fd = dup2(oldfd, newfd)) == -1) {
        fprintf(stderr, "Error (dup2): \"%s\"\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return fd;
}

/*
*******************************************************************************
*                               Utility Routines                              *
*******************************************************************************
*/

/* Returns pointer to next environment-path.
 * - If no paths remain, NULL is returned.
 * - Upon returning NULL, the paths are reset.
 * - Do not attempt to free returned string!
*/
char *nextPath () {
    static int i = 0;
    static int n = 0;

    // Initialize environment-path if necessary.
    if (envp == NULL) {
        initEnvPaths();
    }

    // If past last string. Return NULL and reset.
    if (n >= pathCount) {
        n = i = 0;
        return NULL;
    }

    // Otherwise save offset, increment i and n for next access.
    int offset = i;
    i += strlen(envp + i) + 1;
    n++;

    return envp + offset;
}

/* Free's allocated environment path */
void freePaths () {
    free(envp);
}
