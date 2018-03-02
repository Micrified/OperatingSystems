#if !defined (LIST_H)
#define LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
*******************************************************************************
*                                Data Types                                   *
*******************************************************************************
*/

/* Node types */
typedef enum {
    TYPE_WORD,
    TYPE_REDIR_IN,
    TYPE_REDIR_OUT,
    TYPE_PIPE,
    TYPE_AMP,
    TYPE_SCOLON
} type;

/* Linked list node */
typedef struct node {
    type t;
    const char *sp;
    struct node *next;
} node;

/*
*******************************************************************************
*                             Routine Prototypes                              *
*******************************************************************************
*/

/* Allocates operator node and appends to token-list. */
node *appendOperator (type op);

/* Allocates word node, copies string, and appends to token-list. */
node *appendWord (const char *sp);

/* Free's the token-list. Resets head pointer. */
void freeTokenList (void);

/* Prints the token-list */
void printTokenList (void);

#endif