/*
 * TCB Queue Implementation header file
 * Based on textbook code
 * @author Steven Ha 2019
 */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "rtos.h"

typedef struct {
	tcb_t *p_head;
	tcb_t *p_tail;
} tcb_queue_t;

// Initialize empty queue
inline void tcbQueueInit(tcb_queue_t *queue);

// Initialize queue with a TCB
inline void tcbQueueInit1(tcb_queue_t *queue, tcb_t *tcb);

// Check if queue is empty
inline bool tcbQueueEmpty(tcb_queue_t *queue);

// Get head of queue
inline tcb_t* tcbQueueHead(tcb_queue_t *queue);

// Enqueue a TCB from the queue
inline void tcbQueueEnqueue(tcb_queue_t *queue, tcb_t *tcb);

// Dequeue a TCB from the queue
inline tcb_t* tcbQueueDequeue(tcb_queue_t *queue);
