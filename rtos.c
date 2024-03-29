/*
 * rtos implementation.
 * @author Steven Ha, Benjamin Duo 2019
 */
#include <LPC17xx.h>
#include <stdio.h>
#include "rtos.h"
#include "context.h"
#include "tcb_queue.h"

tcb_t tcb[MAX_TASKS];												// Array of TCBs
const uint32_t *MSP_BASE = (uint32_t *)0x0; // Base address of the main stack

tcb_t *currTCB = NULL;											// TCB of current task
tcb_t *nextTCB = NULL;											// TCB of next task
uint8_t taskCount = 0;

tcb_queue_t waitingQueue;										// Queue of tasks that waiting
tcb_queue_t readyQueues[MAX_TASKS];					// Ready queue
uint32_t readyQueueBitVector = 0;						// Vector representation of ready queue

uint32_t msTicks = 0;
bool kernalStart = false;

// Initialize RTOS
void osInit(){		
	// Configure Systick
	SysTick_Config(SystemCoreClock/1000);
	
	// Intialize each TCB with the base address of each stack
	// Work from the base adrress of the main stack
	int8_t i;
	for(i = MAX_TASKS - 1; i >= 0; i--){
		if(i == MAX_TASKS - 1){
			tcb[i].stackPtr = (uint32_t *)(*MSP_BASE - 2000);
		}
		else{
			tcb[i].stackPtr = (uint32_t *)((uint32_t)(tcb[i+1].stackPtr) - 1000);
		}
		tcb[i].id = i;
		tcbQueueInit(&readyQueues[i]); // Init readyQueue
	} 
	
	// Copy main stack to process stack of new main() task
	uint32_t stackSize = *MSP_BASE - __get_MSP();
	uint32_t j;
	uint32_t *mainPtr;
	
	for(j = 0; j <= stackSize; j++){
		if (j != 0){
			tcb[MAIN_TASK].stackPtr = (uint32_t *)((uint32_t)(tcb[MAIN_TASK].stackPtr) - 1);
		}		
			mainPtr = (uint32_t *)(*MSP_BASE - j); // Getting content from main stack
		*(tcb[MAIN_TASK].stackPtr) = *mainPtr; //
	}
	
	// Set MSP to the Main stack base address
	__set_MSP(*MSP_BASE);
	
	// Switch from MSP to PSP
	uint32_t controlReg = __get_CONTROL();
	controlReg |= (1 << 1);
	__set_CONTROL(controlReg);
	
	// Set PSP to address of the top of the stack of the main task
	__set_PSP((uint32_t) tcb[MAIN_TASK].stackPtr);
	
	// Update task count, set main task priority & current task index
	taskCount += 1;
	tcb[MAIN_TASK].priority = DEFAULT_PRIORITY;
	tcb[MAIN_TASK].state = RUNNING;
	currTCB = &tcb[MAIN_TASK];
	
	// Create idle task
	osCreateTask(idleTask, NULL, 0);
}

void osKernalStart(void){
	kernalStart = true;
}
	
// Create Task/Thread
bool osCreateTask(rtosTaskFunc_t func, void* args, uint8_t priority){
	// Determine if there is an available TCB for the new task
	// Return false if there are no tasks available
	if (taskCount == MAX_TASKS){
		return false;
	}
	tcb[taskCount].id = taskCount;
	// printf("Task id: %d\n", tcb[taskCount].id);
	
	// Set priority of new task
	if (priority > MAX_PRIORITY){
		priority = DEFAULT_PRIORITY;
	}
	tcb[taskCount].priority = priority;
	
	tcb[taskCount].p_next_tcb = NULL;
	
	// Set state to READY. Enqueue task into ready queue. Update queue vector
	tcb[taskCount].state = READY;
	tcbQueueEnqueue(&readyQueues[priority], &tcb[taskCount]);
	readyQueueBitVector |= (1 << tcb[taskCount].priority);
	
	// Intialize stack of the task
	// Set PSR
	tcb[taskCount].stackPtr = (uint32_t *)((uint32_t)(tcb[taskCount].stackPtr) - 4);
	*tcb[taskCount].stackPtr = 0x01000000;
	// Set PC (address of task function)
	tcb[taskCount].stackPtr = (uint32_t *)((uint32_t)(tcb[taskCount].stackPtr) - 4);
	*tcb[taskCount].stackPtr = (uint32_t) func;
	// Set R0 (argument of task)
	tcb[taskCount].stackPtr = (uint32_t *)((uint32_t)(tcb[taskCount].stackPtr) - 24);
	*tcb[taskCount].stackPtr = (uint32_t) args;
	// Move stack pointer to R4
	tcb[taskCount].stackPtr = (uint32_t *)((uint32_t)(tcb[taskCount].stackPtr) - 32);

	// Update task count
	taskCount += 1;
	
	return true;
}


// Thread yields to next task
inline void osTaskYield(void){
	SCB->ICSR |= (1 << 28);
}


// Delay a task for n milliseconds
void osDelay(uint32_t delay){
		currTCB->period = delay;	// Save the delay
		currTCB->prev = msTicks;	// Save current time
		currTCB->state = WAITING;
		tcbQueueEnqueue(&waitingQueue, currTCB);
		SCB->ICSR |= (1 << 28);		// Call scheduler
}


// Calls scheduler after time slice
void SysTick_Handler(void){
	msTicks++;
	
	// Run scheduler if there are more than one task
	if(kernalStart){	
			// Call PendSV_Handler to perform the context switch
			SCB->ICSR |= (1 << 28);	
	}
}


// Conext switching
void PendSV_Handler(void){
	// Push PSR, PC, LR, R12, R0-R3 contents onto the current task's stack (Automatically done when handler starts)
	// Push R4-R11 onto the current task's stack
	// Record the current stack pointer in the current task's TCB
	currTCB->stackPtr = (uint32_t *)(storeContext());
	
	// If task is still a running task, enqueue it into the ready queue
	if(currTCB->state == RUNNING){
		tcbQueueEnqueue(&readyQueues[currTCB->priority], currTCB);
		readyQueueBitVector |= (1 << currTCB->priority);
		currTCB->state = READY;																				// Set state of current task to ready
	}
	
	// Check if there is a task waiting to be run
	bool waitTCBFound = false;

	if(!tcbQueueEmpty(&waitingQueue)){		
		nextTCB = tcbQueueHead(&waitingQueue);
		tcb_t *tempTCB = NULL;
		
		while(!waitTCBFound){
			if((uint32_t)(msTicks - nextTCB->prev) >= nextTCB->period){
				nextTCB->prev += nextTCB->period;
				
				// Remove the TCB from the waiting queue
				if(nextTCB == tcbQueueHead(&waitingQueue)){
					tcbQueueDequeue(&waitingQueue);
				}
				else{
					tempTCB->p_next_tcb = nextTCB->p_next_tcb;
					if(tempTCB->p_next_tcb == NULL){	// if nextTCB is the tail, update tail
						waitingQueue.p_tail = tempTCB;
					}
				}
				nextTCB->p_next_tcb = NULL;
				waitTCBFound = true;
			}
			else{
				tempTCB = nextTCB;
				nextTCB = nextTCB->p_next_tcb;
				if(nextTCB == NULL){ // Exit loop if no task is ready
					break;
				}
			}
		}
	}
	
	// Look in ready queue if no delayed task is ready
	if(!waitTCBFound){
		// Dequeue next task from priority queue to run
		uint8_t topPriority = 31 - __CLZ(readyQueueBitVector); // Get index of next non-empty queue
		nextTCB = tcbQueueDequeue(&readyQueues[topPriority]);

		if(tcbQueueEmpty(&readyQueues[topPriority])){ 								// set bit to 0 if the queue is empty
			readyQueueBitVector &= ~(1 << topPriority);
		}
	}
	nextTCB->state = RUNNING; 																		// Set state of the next task to running
	
	// Set current TCB to next TCB
	currTCB = nextTCB;
	
	// Load the next task's top of stack address into the stack pointer
	// Pop R4-R11 from the next task's stack
	restoreContext((uint32_t)(nextTCB->stackPtr));

	// Pop PSR, PC, LR, R12, R0-R3 contents from the next task's stack (Automatically happens when handler exits)
}


// Idle Task
void idleTask(void *args){
	while(true);
}


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
			search = search->p_next_tcb;
	} 
	if(search == queue->p_head){ //task is the head
		if(search->p_next_tcb == NULL){ //task is the only task in the queue
			tcbQueueDequeue(queue);
		}
		else if(queue->p_head->p_next_tcb != NULL){ // there is 1 or more tasks behind it
			queue->p_head = queue->p_head->p_next_tcb;
		}
	} 
	else if(search->p_next_tcb == NULL){ //task is at the end of the queue
			past->p_next_tcb = NULL;
			queue->p_tail = past;
	}
	else if(search != queue->p_head && search != queue->p_tail){//between two tasks
			past->p_next_tcb = search->p_next_tcb;
			
	}
	else{
		return osError;
	}
	task->p_next_tcb = NULL;
	return osOK;	
}


// Create Mutex
void init_Mutex(osMutex *m){
    m->owner = NULL;
    m->mutex = 1; //1 is unlocked
	
		// Initialize waitLists
		uint8_t i;
		for(i = 0; i <= MAX_PRIORITY; i++){
			tcbQueueInit(&m->waitList[i]);
		}	
		m->waitListBitVector = 0;
		
    m->ownerPriority = 0;
}


// Acquire Mutex
void osAcquireMutex(osMutex *m){ //ignoring timeouts for simplicity
	__disable_irq();
    
	while(m->mutex == 0){ // Mutex is locked
		currTCB->state = WAITING;								//Change task to waiting
		tcbQueueEnqueue(&m->waitList[currTCB->priority], currTCB); //Place into waitList
		m->waitListBitVector |= (1 << currTCB->priority);
		
		// Perform priority inheritance
		if(currTCB->priority > m->ownerPriority){
				tcbMutexDequeue(&readyQueues[m->ownerPriority], m->owner); 	// Remove owner task from its priority queue
				if(tcbQueueEmpty(&readyQueues[m->ownerPriority])){
					readyQueueBitVector &= ~(1 << m->ownerPriority);
				}	
				m->owner->priority = currTCB->priority;  										// upgrade priority
				tcbQueueEnqueue(&readyQueues[currTCB->priority], m->owner); // move to current highest priority queue
				readyQueueBitVector |= (1 << m->ownerPriority);
		}   
		
		__enable_irq();
		osTaskYield();
		__disable_irq();
	}
	
	if(m->mutex == 1){ //if mutex is free
		m->owner = currTCB;
		m->mutex = 0;
		m->ownerPriority = currTCB->priority;	// Save owner's priority for priority inheritance
				
		__enable_irq();
	}
}


//Release Mutex
void osReleaseMutex(osMutex *m){
	__disable_irq();
	
	// Release mutex if mutex is locked and the current task is the owner
	if(m->mutex == 0 && currTCB == m->owner){
		m->mutex = 1; //Unlock mutex
		currTCB->priority = m->ownerPriority; // Reset priority of the owner
						
		m->owner = NULL;
		
		// Remove highest priority blocked task and place it into the ready queue
		uint8_t topPriority = 31 - __CLZ(m->waitListBitVector);
		tcb_t *unblocked = tcbQueueDequeue(&m->waitList[topPriority]);
		
		if(tcbQueueEmpty(&m->waitList[topPriority])){
			m->waitListBitVector &= ~(1 << topPriority);
		}
		
		tcbQueueEnqueue(&readyQueues[topPriority], unblocked);
		readyQueueBitVector |= (1 << unblocked->priority);
		unblocked->state = READY;		
	}
	else{
		printf("Task %d doesn't own mutex\n", currTCB->id);
	}

	__enable_irq();
}


//Initialize Semaphore
void init_Sema(osSem_Id *s, int32_t n){
    s->count = n;
    tcbQueueInit(&s->waitList);
}

//Wait Semaphore
void osSemWait(osSem_Id *s){
    __disable_irq();
    (s->count)--;

    if(s->count >= 0){
      __enable_irq();
    }
    else{
			tcbQueueEnqueue(&(s->waitList), currTCB);
			currTCB->state = WAITING;
      __enable_irq();
      osTaskYield();
		}
}


//Signal Semaphore
void osSemSignal(osSem_Id *s){
    __disable_irq();
		(s->count)++;
	
    if(s->count <= 0){
        tcb_t *shift = s->waitList.p_head;
        if(shift != NULL){ //move all tasks being blocked into the correct priority ready queue
          shift = tcbQueueDequeue(&s->waitList);  
					tcbQueueEnqueue(&(readyQueues[shift->priority]), shift);
					readyQueueBitVector |= (1 << shift->priority);
					shift->state = READY;
        }
    }
    __enable_irq();
}


__asm void osEnterFunc(void){
	PUSH 	{R4-R11}
	BX		LR
}

__asm void osExitFunc(void){
	POP 	{R4-R11}
	BX		LR
}


void osGetPriority(void){
	printf("Priority of current task: %d\n", currTCB->priority);
}
