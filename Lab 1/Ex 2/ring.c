#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*
 *************************************************************************
 *                                Ex.2                                   *
 * Author(s):   Barnabas Busa, Joe Jones, Charles Randolph.              *
 *************************************************************************
*/ 

#define N_MAX       50      // Counting limit (incl).
#define P_MAX       16      // Maximum process count.
#define D_MAX       3       // Maximum allowed digits.

#define FD_W(p,pmax)        (((((p) + 1) * 2) + 1) % (2 * (pmax)))
#define FD_R(p,pmax)        (((p) * 2) % (2 * (pmax)))

int n, p, pmax;             // Count (n), process num (p), max process (pmax).
int mpid, fds[2 * P_MAX];   // Master pid (mpid), file descriptors (fds).
char b[D_MAX];              // Message buffer (b).

/* Closes all file descriptors besides those specified for reading/writing. */
void closeExcept (int r, int w) {
    for (int i = 0; i < 2 * pmax; i++) {
        if (i == r || i == w) continue;
        close(fds[i]);
    }
}

int main (int argc, const char *argv[]) {
    mpid = getpid();
    pmax = atoi(argv[1]);

    // Input validation.
    if (pmax < 1 || pmax > P_MAX) {
        fprintf(stderr, "Error: Specify p such that 0 < p <= P_MAX!\n");
        exit(-1);
    }

    // Initialize all pipes.
    for (p = 0; p < pmax; p++) {
        pipe(fds + 2 * p);
    }

    // Create all forks with their process number.
    for (p = 1; p < pmax; p++) {
        if (fork() == 0) break;
    }

    // Ensure master-process has (p = 0).
    p = (getpid() == mpid) ? 0 : p;

    // Close all file-descriptors except those this process -
    // will read and write from.
    closeExcept(FD_R(p, pmax), FD_W(p, pmax));

    // If master process, kick off the ring.
    if (getpid() == mpid) {
        printf("pid=%d: 0\n", mpid);
        sprintf(b, "%d", 1);
        write(fds[FD_W(p, pmax)], b, D_MAX);
    }

    // All processes: (while n <= N_MX)
    // 1. Poll the reading file-descriptor for a number.
    // 2. Print acquired number.
    // 3. Incremented and send acquired number to next process.
    for (int bytes_read = 0; n <= N_MAX; ) {
        if ((bytes_read = read(fds[FD_R(p, pmax)], b, D_MAX)) == D_MAX) {

            if ((n = atoi(b)) <= N_MAX) {
                printf("pid=%d: %d\n", getpid(), n);
            }
            sprintf(b, "%d", n + 1);
            write(fds[FD_W(p, pmax)], b, D_MAX);
        } else {
            break;
        }
    }
    return 0;
}