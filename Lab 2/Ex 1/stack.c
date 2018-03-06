#include "stack.h"

/*
*******************************************************************************
*                            Symbolic Constants                               *
*******************************************************************************
*/

#define DEFAULT_STACK_SIZE  32

/*
*******************************************************************************
*                             Global Variables                                *
*******************************************************************************
*/

/* Item stack */
Item **stack;

/* Stack pointer, Stack size */
unsigned sp, sz;

/*
*******************************************************************************
*                              Item Routines                                  *
*******************************************************************************
*/

/* Allocates a copy of the given string. String must be null-terminated. */
char *copyString (const char *s) {
    char *copy = malloc(strlen(s) + 1);
    assert(copy != NULL && "Error: Couldn't allocate string!\n");
    return strcpy(copy, s);
}

/* Allocates a new item with the given type and word. Returns pointer */
Item *newItem (unsigned type, char *word) {
    Item *item = malloc(sizeof(Item));
    assert(item != NULL && "Error: Couldn't allocate item!\n");
    *item = (Item){.type = type, .word = word};
    return item;
}

/* Free's an item */
void freeItem (Item *item) {
    if (item == NULL) {
        return;
    }
    if (item->word != NULL) {
        free(item->word);
    }
    free(item);
}

/* [DEBUG] Prints an item */
void printItem (Item *item) {
    if (item == NULL) {
        fprintf(stdout, "[ NULL ]\n");
        return;
    }
    fprintf(stdout, "[ TYPE: %u | WORD: \"%s\"]\n", item->type, item->word);
}

/*
*******************************************************************************
*                              Stack Routines                                 *
*******************************************************************************
*/

/* Reallocates the stack to the given size. Updates the size if successful. */
static void resizeStack (unsigned size) {

    if (size <= sz) {
        return;
    }

    if (stack == NULL) {
        stack = malloc(size * sizeof(Item *));
    } else {
        stack = realloc(stack, size * sizeof(Item *));
    }
    assert(stack != NULL && "Error: Stack Allocation/Reallocation failure!\n");
    sz = size;
}

/* Pushes item onto stack. Resizes stack if necessary */
void push (Item *item) {
    if (sp >= sz) {
        resizeStack(MAX(DEFAULT_STACK_SIZE, 2 * sz));
    }
    stack[sp++] = item;
}

/* Pops item and returns pointer. If nothing on stack, NULL is returned */
Item *pop () {
    if (sp <= 0) {
        return NULL;
    }
    return stack[--sp];
}

/* Returns the top item on the stack. If nothing on stack, NULL is returned */
Item *peek() {
    if (sp <= 0) {
        return NULL;
    }
    return stack[sp - 1];
}

/* Flushes the stack by freeing all items. */
void flush () {
    while (pop() != NULL)
        ;
}

/* Free's the stack (warning: destroys all items) */
void freeStack (void) {
    while (sp > 0) {
        freeItem(pop());
    }
    free(stack);
    stack = NULL;
}

/* [DEBUG] Prints the stack */
void printStack (void) {
    for (int p = sp - 1; p >= 0; p--) {
        printItem(stack[p]);
    }
}




