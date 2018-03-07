#include "queue.h"

/*
*******************************************************************************
*                                 Data Types                                  *
*******************************************************************************
*/

/* Node for linked-list */
typedef struct node {
    Item item;
    struct node *next;
} Node;

/*
*******************************************************************************
*                             Global Variables                                *
*******************************************************************************
*/

/* Queue base pointer, head pointer. */
Node *head, *tail;

/* Queue size */
unsigned size;

/*
*******************************************************************************
*                              Node Routines                                  *
*******************************************************************************
*/

/* Allocates and returns a new node configured with the given parameters */
static Node *newNode (Item item, Node *next) {
    Node *node = malloc(sizeof(Node));
    assert(node != NULL && "Error: Couldn't allocate node!\n");
    *node = (Node){.item = item, .next = next};
    return node;
}

/* Free's the given node (and item) */
static void freeNode (Node *node) {
    free(node);
}

/*
*******************************************************************************
*                              Queue Routines                                 *
*******************************************************************************
*/

/* Enqueues item. Resizes queue if necessary */
void enqueue (Item item) {
    assert(!((tail == NULL) ^ (head == NULL)) && "Error: Queue corruption!");
    if (tail == NULL) {
        head = tail = newNode(item, NULL);
    } else {
        tail = (tail->next = newNode(item, NULL));
    }
    size++;
}

/* Dequeues an item. Returns NULL_ITEM if empty. */
Item dequeue () {
    Item item = NULL_ITEM;
    if (head != NULL) {
        item = head->item;
        Node *newHead = head->next;
        freeNode(head);
        head = newHead;
        size--;
    }
    // If queue emptied, reset the tail pointer.
    if (head == NULL) {
        tail = NULL;
    }
    return item;
}

/* Flushes the queue [Warning: Destroys all queued items] */
void flush () {
    while (!IS_NULL(dequeue()))
        ;
}

/* Returns the lead item on the queue. If empty, NULL is returned */
Item getHead () {
    return (head == NULL) ? NULL_ITEM : head->item;
}

/* Returns the tail item on the queue. If empty, NULL is returned */
Item getTail () {
    return (tail == NULL) ? NULL_ITEM : tail->item;
}

/* Returns the size of the queue */
unsigned queueSize() {
    return size;
}

