#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAX_ARG_LEN             100

int nargs (const char *cmd, char delim) {
    int n = 0; 
    while (*cmd != '\0') { n += (*cmd == delim); cmd++; }
    return n + 1;
}

char **allocStrings (int n) {
    char **args = NULL;
    if ((args = calloc(n, sizeof(char *))) == NULL) {
        fprintf(stderr, "Error: Couldn't allocate arguments!\n");
        exit(EXIT_FAILURE);
    }
    return args;
}

void dropWhile (char x, char **s) {
    while (**s == x) (*s)++;
}

void dropWhileNot (char x, char **s) {
    while (**s != x) (*s)++;
}

char **getArgs (int n, char *cmd, char **args) {

    for (int i = 0; i < n; i++) {
        dropWhile(' ', &cmd);
        args[i] = cmd;
        dropWhileNot(' ', &cmd);
        *(cmd++) = '\0';
    }

    return args;
}

char **getPaths (int n, char *path, char **paths) {
    for (int i = 0; i < n; i++) {
        paths[i] = path;
        dropWhileNot(':', &path);
        *(path++) = '\0';
    }
    return paths;
}

char *stringFromStrings (char *a, char *b) {
    int n = strlen(a) + strlen(b) + 1;
    char *s = malloc(n * sizeof(char));
    sprintf(s, "%s/%s", a, b);
    return s;
}

int main (int argc, char *argv[]) {
    char *cmd = argv[1];
    int n = nargs(cmd, ' ');
    char **args = allocStrings(n + 1);

    // Split Arguments.
    args = getArgs(n, cmd, args);

    //printf("execve(\"%s\",[", cmd);
    //for (int i = 0; i < n + 1; i++) {
    //    printf("\"%s\"%c", (args[i] == NULL ? "NULL" : args[i]), (i < n) ? ',' : ']');
    //}
    //printf(", NULL);\n");

    // Attempt to execute without looking at the environment paths.
    execve(cmd, args, NULL);

    // If we get here, that failed. So try at each path.
    char *path = getenv("PATH");
    //printf("path = %s\n", path);
    int p = nargs(path, ':');
    //printf("There are %d paths.\n", p);
    char **paths = allocStrings(p);
    getPaths(p, path, paths);

    for (int i = 0; i < p; i++) {
        //printf("%d = %s\n", i, paths[i]);
        char *newPath = stringFromStrings(paths[i], args[0]);
        //printf("Trying exec with new path: \"%s\"\n", newPath);
        execve(newPath, args, NULL);
    }

    fprintf(stderr, "Error: Command %s not found!\n", args[0]);

    return 0;
}