/*
 * TCB Queue Implementation header file
 * Based on textbook code
 * @author Steven Ha 2019
 */
#ifndef __TCB_QUEUE_H
#define __TCB_QUEUE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "rtos.h"

typedef struct {
	tcb_t *p_head;
	tcb_t *p_tail;
} tcb_queue_t;

// Initialize empty queue
void tcbQueueInit(tcb_queue_t *queue);

// Initialize queue with a TCB
void tcbQueueInit1(tcb_queue_t *queue, tcb_t *tcb);

// Check if queue is empty
bool tcbQueueEmpty(tcb_queue_t *queue);

// Get head of queue
tcb_t* tcbQueueHead(tcb_queue_t *queue);

// Get tail of queue
tcb_t* tcbQueueTail(tcb_queue_t *queue);

// Enqueue a TCB from the queue
void tcbQueueEnqueue(tcb_queue_t *queue, tcb_t *tcb);

// Dequeue a TCB from the queue
tcb_t* tcbQueueDequeue(tcb_queue_t *queue);

#endif
