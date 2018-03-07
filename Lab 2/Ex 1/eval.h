#if !defined(EVAL_H)
#define EVAL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "queue.h"
#include "iwish.tab.h"
#include "strtab.h"
#include "util.h"

/*
*******************************************************************************
*                                Eval Routines                                *
*******************************************************************************
*/

/* Evaluates all items on the queue (I.E. command execution) */
void evalQueue ();

#endif