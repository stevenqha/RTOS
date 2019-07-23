/*
 * rtos implementation.
 * @author Steven Ha, Benjamin Duo 2019
 */
#include "rtos.h"


//dequeue tasks in the TCB functions, enqueue is the same
inline void tcbMutexDequeue(tcb_queue_t *queue, tcb_t* task){
    tcb_t* past = NULL;
    tcb_t* search = NULL;

    if(queue->p_head == NULL){ 
		return NULL;
	}

    search = queue->p_head;

    while(search != task){ //find where the task is
        past = search;
        search=search->p_next_tcb;
    }
    if(search->p_next_tcb == NULL){ //task is at the end of the queue
        past->p_next_tcb = NULL;
        p_tail = past;
    }
    if(search == queue->p_head){ //task is the head
        queue->p_head = queue->p_head->p_next_tcb;
        if(queue->p_head == NULL){
            queue->p_tail = NULL;
        }
    }
    if(search)

}


//define osStatus
typedef bool osStatus;
#define osOK 1
#define osError 0

//Define Mutex
typedef struct {
    bool mutex;
    tcb_queue_t waitList;
    tcb_t *owner;
    uint8_t priority;
    uint8_t h_prior= 0; //Keep track of highest priority being blocked
}osMutex;


//Create Mutex
void init_Mutex(osMutex *m){
    m->owner = NULL;
    m->mutex = 1; //1 is unlocked
    tcbQueueInit(&(m->waitList));
    m->priority = 0;
}

//Acquire Mutex
osStatus osAcquireMutex(osMutex *m){ //ignoring timeouts for simplicity
	__disable_irq();
    
    if(m->mutex == 1){
        m->owner = currTCB;
        if(currTCB->priority < m->h_prior){
            m->priority = currTCB->priority;
            currTCB->priority = m->h_prior; //set equavialent to the current highest priority 

            //functions to move the task

            tcbQueueEnqueue(&readyQueues[h_prior],m->owner);
        }
        else
        {
            m->h_prior = currTCB->priority;
            m->priority = currTCB->priority;
        }
        
        m->mutex = 0;
        __enable_irq();

        return osOK;
    }
    else
    {
        //block task
        if(currTCB->priority > m->h_prior){
            m->h_prior = currTCB->priority; 
            m->owner->priority = m->h_prior; 
        }    
        __enable_irq();
        return osError;
    }
};

//Release Mutex
osStatus osReleaseMutex(osMutex *m){
	__disable_irq();

    if(m->mutex == 0 && currTCB == m->owner){
        m->mutex = 1;
        currTCB->priority=m->priority;
        m->owner = NULL;
        

        __enable_irq();

        return osOK;
    }
    else
    {   
        __enable_irq();
        return osError;
    }
    
};

//Define Semaphore
typedef struct{
    uint32_t count = 0;
    tcb_queue_t waitList;
}osSem_Id;

//Initialize Semaphore
void init_Sema(osSem_Id *s, uint32_t n){
    s->count = n;
}

//Wait Semaphore
void osSemWait(osSem_Id *s){
    __disable_irq();
    (s->count)--;

    if(s->count < 0)
    {
        //block task
        __enable_irq();
    }
    else
    __enable_irq();
}

//Signal Semaphore
void osSemSignal(osSem_Id *s){
    __disable_irq();
    (s->count)++;
    if(s->count <= 0)
    {
        //unblock task
    }
    __enable_irq();
}