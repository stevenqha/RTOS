/*
 * rtos implementation.
 * @author Steven Ha, Benjamin Duo 2019
 */
#include "rtos.h"


//define osStatus
typedef bool osStatus;
#define osOK 1
#define osError 0

//dequeue tasks in the TCB functions, enqueue is the same
inline osStatus tcbMutexDequeue(tcb_queue_t *queue, tcb_t* task){
    tcb_t* past = NULL;
    tcb_t* search = NULL;

    if(queue->p_head == NULL){ //exit if queue is empty
		return osError;
	}

    search = queue->p_head;

    while(search != task){ //find where the task is
        past = search;
        search=search->p_next_tcb;
    } 
    if(search == queue->p_head){ //task is the head
        if(search->p_next_tcb == NULL){ //task is the only task in the queue
            queue->p_head = NULL;
            queue->p_tail = NULL;
            return osOK;
        }
        else if(queue->p_head->p_next_tcb != NULL){ // there is 1 or more tasks behind it
            queue->p_head = queue->p_head->p_next_tcb;
            return osOK;
        }
    } 
    if(search->p_next_tcb == NULL){ //task is at the end of the queue
        past->p_next_tcb = NULL;
        queue->p_tail = past;
        return osOK;
    }
    if(search != queue->p_head && search != queue->p_tail){//between two tasks
        past->p_next_tcb = search->p_next_tcb;
        return osOK;
    }
    return osError;
}

//Define Mutex
typedef struct {
    bool mutex;
    tcb_queue_t waitList;
    tcb_t *owner;
    uint8_t priority;
    uint8_t h_prior; //Keep track of highest priority being blocked
}osMutex;


//Create Mutex
void init_Mutex(osMutex *m){
    m->owner = NULL;
    m->mutex = 1; //1 is unlocked
    tcbQueueInit(&(m->waitList));
    m->priority = 0;
    h_prior= 0; 
}

//Acquire Mutex
osStatus osAcquireMutex(osMutex *m){ //ignoring timeouts for simplicity
	__disable_irq();
    
    if(m->mutex == 1){ //if mutex is free
        m->owner = currTCB;
        if(currTCB->priority < m->h_prior){
            m->priority = currTCB->priority;
            currTCB->priority = m->h_prior; //set equavialent to the current highest priority 

            tcbMutexDequeue(&readyQueues[m->priority],m->owner); //remove task from its priority queue
            tcbQueueEnqueue(&readyQueues[h_prior],m->owner); //move to current highest priority queue
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
        tcbQueueEnqueue(&(m->waitList), currTCB); //block task/place into waitList

        if(currTCB->priority > m->h_prior){
            m->h_prior = currTCB->priority; 
            m->owner->priority = m->h_prior; 
            tcbMutexDequeue(&readyQueues[m->priority],m->owner); //remove task from its priority queue
            tcbQueueEnqueue(&readyQueues[h_prior],m->owner); //move to current highest priority queue
        }    
        __enable_irq();
        return osError;
    }
};

//Release Mutex
osStatus osReleaseMutex(osMutex *m){
	__disable_irq();

    if(m->mutex == 0 && currTCB == m->owner){
        m->mutex = 1; //free mutex
        currTCB->priority=m->priority;
                
        tcbMutexDequeue(&readyQueues[h_prior],m->owner); //remove task from highest priority queue
        tcbQueueEnqueue(&readyQueues[m->priority],m->owner); //move to current previous priority queue
        m->owner = NULL;
        
        if(!tcbQueueEmpty(%(m->waitList))){
            uint8_t next_prior;
            tcb_t *shift, *high;
            shift = m->waitList->p_head;
            next_prior = 0; //lowest priority

            while(shift != NULL){
                if(shift->priority > next_prior){ //find the highest priority task
                    next_prior = shift->priority;
                    high = shift;
                }
                shift = shift->p_next_tcb;
            }
            tcbMutexDequeue(%(m->waitList), high); 
            tcbQueueEnqueue(%(readyQueues[next_prior]),high); //change the h.p. task from blocked to ready
            h_prior = next_prior; //update the newest highest priority
        }

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
    uint32_t count;
    tcb_queue_t waitList;
}osSem_Id;

//Initialize Semaphore
void init_Sema(osSem_Id *s, uint32_t n){
    s->count = n;
    tcbQueueInit(&(s->waitList));
}

//Wait Semaphore
void osSemWait(osSem_Id *s){
    __disable_irq();
    (s->count)--;

    if(s->count < 0)
    {
        tcbQueueEnqueue(&(s->waitList), currTCB);
        __enable_irq();
    }
    else
    __enable_irq();
}

//Signal Semaphore
void osSemSignal(osSem_Id *s){
    __disable_irq();
    if(s->count <= 0)
    {
        tcb_t *shift = s->waitList->p_head;
        while(shift != NULL){ //move all tasks being blocked into the correct priority ready queue
            tcbQueueEnqueue($(readyQueues[shift->priority]),shift);
            tcbMutexDequeue($(s->waitList), shift);
            shift = shift->p_next_tcb;
            (s->count)++;
        }
        s->waitList->p_head = NULL;
        s->waitList->p_tail - NULL;
    }
    __enable_irq();
}