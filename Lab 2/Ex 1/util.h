#if !defined(UTIL_H)
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

/*
*******************************************************************************
*                           Syscall Utility Routines                          *
*******************************************************************************
*/

/* Closes file-descriptor. Aborts with message on error */
void safeClose (int fd);

/* Opens file with flags. Aborts with message on error */
int safeOpen (const char *fileName, int flags);

/* Opens file with flags and mode. Aborts with message on error */
int safeOpenMode (const char *fileName, int flags, mode_t mode);

/* Opens pipe with given buffer. Aborts with message on error */
void safePipe(int pipefds[2]);

/* Copies a file descriptor. Aborts with message on error */
int safeDup2 (int oldfd, int newfd);

/*
*******************************************************************************
*                             Path Utility Routines                           *
*******************************************************************************
*/

/* Returns pointer to next environment-path.
 * - If no paths remain, NULL is returned.
 * - Upon returning NULL, the paths are reset.
 * - Do not attempt to free returned string!
*/
char *nextPath ();

/* Free's allocated environment path */
void freePaths (); 

#endif
