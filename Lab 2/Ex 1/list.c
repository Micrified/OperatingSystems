#include "list.h"

/*
*******************************************************************************
*                              Global Variables                               *
*******************************************************************************
*/

// Head and tail of linked-list.
node *listHead, *listTail;

/*
*******************************************************************************
*                              Internal Routines                              *
*******************************************************************************
*/

/* Allocates a new node. Assigns given type and string pointer. */
static node *newNode (type t, const char *sp) {
    node *n;
    if ((n = malloc(sizeof(node))) == NULL) {
        fprintf(stderr, "Error: Couldn't allocate node!\n");
        exit(EXIT_FAILURE);
    }
    n->t = t; n->sp = sp; n->next = NULL;
    return n;
}

/* Allocates a copy of a given string and returns the pointer. */
static char *newString (const char *sp) {
    char *copy;
    if ((copy = malloc(strlen(sp) + 1)) == NULL) {
        fprintf(stderr, "Error: Couldn't allocate string!\n");
        exit(EXIT_FAILURE);
    }
    return strcpy(copy, sp);
}

/* Free's a node (and children first) */
static void freeNode (node *n) {
    if (n == NULL) {
        return;
    }
    free(n->next);
    free(n);
}

/* Prints a node (and children second) */
void printNode (node *n) {
    if (n == NULL) {
        fprintf(stdout, " Null\n");
        return;
    }
    if (n->t == TYPE_WORD) {
        fprintf(stdout, "{ %s } -> ", n->sp);
        goto visitNext;
    }
    if (n->t == TYPE_REDIR_IN) {
        fprintf(stdout, "{ < } -> ");
        goto visitNext;
    }
    if (n->t == TYPE_REDIR_OUT) {
        fprintf(stdout, "{ > } -> ");
        goto visitNext;
    }
    if (n->t == TYPE_PIPE) {
        fprintf(stdout, "{ | } -> ");
        goto visitNext;
    }
    if (n->t == TYPE_AMP) {
        fprintf(stdout, "{ & } -> ");
        goto visitNext;
    } 

    fprintf(stdout, "{ ; } -> ");

    visitNext: 
        printNode(n->next);
}

/*
*******************************************************************************
*                                 Routines                                    *
*******************************************************************************
*/

/* Allocates operator node and appends to token-list. */
node *appendOperator (type op) {
    if (listHead == NULL) {
        return (listHead = (listTail = newNode(op, NULL)));
    }
    return (listTail = (listTail->next = newNode(op, NULL)));
}

/* Allocates word node, copies string, and appends to token-list. */
node *appendWord (const char *sp) {
    if (listHead == NULL) {
        return (listHead = (listTail = newNode(TYPE_WORD, newString(sp))));
    }
    listTail->next = newNode(TYPE_WORD, newString(sp));
    return (listTail = listTail->next);
}

/* Free's the token-list. Resets head pointer. */
void freeTokenList (void) {
    if (listHead == NULL) {
        return;
    }
    freeNode(listHead);
    listHead = NULL;
}

/* Prints the token-list */
void printTokenList (void) {
    printNode(listHead);
}