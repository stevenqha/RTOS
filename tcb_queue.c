/*
 * TCB Queue Implementation header file
 * Based on textbook code
 * @author Steven Ha 2019
 */

#include "tcb_queue.h"

// Initialize empty queue
inline void tcbQueueInit(tcb_queue_t *queue){
	queue->p_head = NULL; 					
	queue->p_tail = NULL; 					
}

// Initialize queue with a TCB
inline void tcbQueueInit1(tcb_queue_t *queue, tcb_t *tcb){
	queue->p_head = tcb; 													
	queue->p_tail = tcb; 													
	tcb->p_next_tcb = NULL; 													
}

// Check if queue is empty
inline bool tcbQueueEmpty(tcb_queue_t *queue){
	return queue->p_head == NULL;
}

// Get head of queue
inline tcb_t* tcbQueueHead(tcb_queue_t *queue){
	return queue->p_head;
}

// Get tail of queue
inline tcb_t* tcbQueueTail(tcb_queue_t *queue){
	return queue->p_tail;
}

// Enqueue a TCB into the queue
inline void tcbQueueEnqueue(tcb_queue_t *queue, tcb_t *tcb){
	if(queue->p_head == NULL){
		queue->p_head = tcb; 																									
	}
	else{
		queue->p_tail->p_next_tcb = tcb;
	}
	queue->p_tail = tcb;
	tcb->p_next_tcb = NULL; 
}

// Dequeue a TCB from the queue
inline tcb_t* tcbQueueDequeue(tcb_queue_t *queue){
	if(queue->p_head == NULL){ 
		return NULL;
	}
	
	tcb_t *dequeued = queue->p_head;	
	queue->p_head = queue->p_head->p_next_tcb;
	if(queue->p_head == NULL){ 
		queue->p_tail = NULL;
	}
	return dequeued;
}
