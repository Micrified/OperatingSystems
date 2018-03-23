/*
utctime.c
*/

#include <stdio.h>
#include <sys/cdefs.h>
#include "namespace.h"
#include <lib.h>
#include <time.h>

time_t utctime (time_t *tp) {
    message m;

    // Perform system call.
    if (_syscall(PM_PROC_NR, UTCTIME, &m) < 0) {
        fprintf(stderr, "Error: do_utctime failed!\n");
        return -1;
    }

    // Extract time (m2_l1 is a long, so recast).
    time_t utime = (time_t)m.m2_l1;

    // If pointer is non-null, save it there too. 
    if (tp != NULL) {
        *tp = utime;
    }

    // Return result.
    return utime;
}
