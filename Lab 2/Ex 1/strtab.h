#if !defined(STRTAB_H)
#define STRTAB_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define NIL         -1

/*
*******************************************************************************
*                               Table Routines                                *
*******************************************************************************
*/

/* Registers given string in a string table. Returns index of string */
int putString (const char *sp);

/* Returns the string at the given index. On invalid index, NULL is returned */
char *getString (int index);

/* Free's the string table */
void freeStringTable ();


#endif