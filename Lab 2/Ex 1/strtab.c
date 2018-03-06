#include "strtab.h"

#define MAX(a,b)        ((a) > (b) ? (a) : (b))
#define MIN_TAB_SIZE    32
/*
*******************************************************************************
*                             Internal Variables                              *
*******************************************************************************
*/

// The string table pointer.
char *strtab;

// The table pointer, and table size.
int tp, tabSize;

/*
*******************************************************************************
*                              Internal Routines                              *
*******************************************************************************
*/

/* Resizes the string table */
void resizeTable (int toSize) {
    if (strtab == NULL) {
        strtab = calloc(toSize, sizeof(char)); // Allocate with zeroed bytes.
    } else {
        strtab = realloc(strtab, toSize * sizeof(char));
    }
    assert(strtab != NULL && "Error: Failed to alloc/realloc string table!\n");
    tabSize = toSize;
}


/*
*******************************************************************************
*                               Table Routines                                *
*******************************************************************************
*/

/* Registers given string in a string table. Returns index of string */
int putString (const char *sp) {
    int offset = tp;
    tp += strlen(sp) + 1;

    if (tp >= tabSize) {
        resizeTable(MAX(MIN_TAB_SIZE, 2 * tabSize));
    }

    sprintf(strtab + offset, "%s", sp);
    return offset;
}

/* Returns the string at the given index. On invalid index, NULL is returned */
char *getString (int index) {
    if (index >= 0 && index < tp) {
        return strtab + index;
    }
    return NULL;
}

/* Free's the string table */
void freeStringTable () {
    tp = tabSize = 0;
    free(strtab);
}
