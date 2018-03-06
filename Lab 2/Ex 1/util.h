#if !defined(UTIL_H)
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

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
char *nextPath ();

/* Free's allocated environment path */
void freePaths (); 

#endif
