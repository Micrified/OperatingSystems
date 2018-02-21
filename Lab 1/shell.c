#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

/*
 *************************************************************************
 *                                Ex.1                                   *
 * Author(s):   Barnabas Busa, Joe Jones, Charles Randolph.              *
 *************************************************************************
*/ 

#define MAX_PATH    2048
#define MAX_ARGS    100
char path[MAX_PATH + 1];
char paths[MAX_PATH + 1];
char *args[MAX_ARGS + 1];

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
    } else {
        strncpy(paths, getenv("PATH"), MAX_PATH);
        ps = ncstr(paths, ':') + 1;
        as = ncstr(argv[1], ' ') + 1;
        slice(argv[1], as, args);
        execve(args[0], args, NULL);
        for (int i = 0; i < ps; i++) {
            sprintf(path, "%s/%s", nextpath(paths), args[0]);
            execve(path, args, NULL);
        }
        fprintf(stderr, "Error: Couldn't execute %s!\n", argv[1]);
        return -1;
    }
    return 0;
}