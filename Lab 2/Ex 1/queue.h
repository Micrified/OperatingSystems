#if !defined(QUEUE_H)
#define QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "strtab.h"

/*
*******************************************************************************
*                             Symbolic Constants                              *
*******************************************************************************
*/

#define MAX(a,b)            ((a) > (b) ? (a) : (b))
#define MIN(a,b)            ((a) < (b) ? (a) : (b))

// The NULL item.
#define NULL_ITEM           ((Item){.type = -1, .widx = -1})

// Is NULL item.
#define IS_NULL(item)       ((item).type == -1)

/*
*******************************************************************************
*                                 Data Types                                  *
*******************************************************************************
*/

/* Token Descriptor Type */
typedef struct {
    int type;       // Type of token.
    int widx;       // Word index in string table.
} Item;

/*
*******************************************************************************
*                              Queue Routines                                 *
*******************************************************************************
*/

/* Enqueues item. Resizes queue if necessary */
void enqueue (Item item);

/* Dequeues an item. Returns NULL_ITEM if empty. */
Item dequeue ();

/* Flushes the queue [Warning: Destroys all queued items] */
void flush ();

/* Returns the lead item on the queue. If empty, NULL_ITEM is returned */
Item getHead ();

/* Returns the tail item on the queue. If empty, NULL_ITEM is returned */
Item getTail ();

/* Returns the size of the queue */
unsigned queueSize();

#endif