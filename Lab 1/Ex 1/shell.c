#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/wait.h>

/*
 *************************************************************************
 *                                Ex.1                                   *
 * Author(s):   Barnabas Busa, Joe Jones, Charles Randolph.              *
 *************************************************************************
*/ 

#define DEFAULT_SIZE    255

/* Path: Single env-path. Paths: Entire env-path. Args: execve param. */
int pathSize, pathsSize, argsSize;
char *path, *paths, **args;

/* Sets the given buffer size and updates size pointer. Init if NULL. */
void resizeBuffer (void **bp, int *sp, int n, int size) {
    if (n <= *sp) return;

    if (*bp == NULL) {
        *bp = malloc(n * size);
    } else {
        *bp = realloc(*bp, n * size);
    }

    assert(*bp != NULL);
    *sp = n;
}

/* Number of occurences of char `c` strings in string `s` */
int ncstr (const char *s, char c) {
    int n = 0;
    while (*s != '\0') {
        if (*s == c) {
            n++;
            while (*s == c) s++;
        } else {
            s++;
        }
    }
    return n;
}

/* Returns the next path in paths by replacing the delimiter */
char *nextpath (char *paths) {
    static int h = 0, t = 0, s;
    while (paths[t] != ':' && paths[t] != '\0') {
        t++;
    }
    paths[t] = '\0';
    s = h;
    h = t += 1;
    return paths + s;
}

/* Slices string into 'n' strings on space. Stores pointers in args */
void slice (char s[], int n, char *args[]) {
    char *h = s, *t = s;
    if (n > 0) {
        while (*t == ' ') t++;
        h = t;
        while (*t != ' ' && *t != '\0') t++;
        *t = '\0';
        *args = h;
        slice(t + 1, n - 1, args + 1);
    }
}

int main (int argc, char *argv[]) {
    int ps, as, status, pid;

    // Fork: If child, transform process. Otherwise wait.
    if ((pid = fork()) != 0) {
        waitpid(pid, &status, 0);

        // Get environment path. Save to buffer.
        char *envp = getenv("PATH");
        resizeBuffer((void **)&paths, &pathsSize, strlen(envp), sizeof(char));
        strcpy(paths, envp);

        // Split paths by colon. ps = number of delimited items.
        ps = ncstr(paths, ':') + 1;

        // Split input in argv[1] on space. as = number of items.
        as = ncstr(argv[1], ' ') + 1;

        // Allocate argument array for execve. Ensure last pointer NULL.
        resizeBuffer((void **)&args, &argsSize, as + 1, sizeof(char *));
        slice(argv[1], as, args);
        args[as] = NULL;

        // Invoke execve for local executable.
        execve(args[0], args, NULL);

        // Try executing it in all paths.
        for (int i = 0; i < ps; i++) {
            char *p = nextpath(paths);
            int len = strlen(p) + strlen(args[0]) + 2; // slash + null-char = 2.
            resizeBuffer((void **)&path, &pathSize, len, sizeof(char));
            sprintf(path, "%s/%s", p, args[0]);
            execve(path, args, NULL);
        }
        fprintf(stderr, "Error: Couldn't execute %s!\n", argv[1]);
        free(path); free(paths); free(args);
        return -1;
    }
    return 0;
}