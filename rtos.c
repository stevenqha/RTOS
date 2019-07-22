/*
 * rtos implementation.
 * @author Steven Ha, Benjamin Duo 2019
 */
#include <LPC17xx.h>
#include "rtos.h"
#include "tcb_queue.h"

tcb_t tcb[MAX_TASKS];												// Array of TCBs
const uint32_t *MSP_BASE = (uint32_t *)0x0; // Base address of the main stack

tcb_t *currTCB;															// TCB of current task
tcb_t *nextTCB;															// TCB of next task
uint8_t taskCount = 0;

tcb_queue_t readyQueues[NUM_PRIORITY];			// Ready queue
uint32_t non_empty_queue_bit_vector = 0;		// Vector representation of ready queue
	
uint32_t msTicks = 0;

// Initialize RTOS
void rtosInit(){		
	// Intialize each TCB with the base address of each stack
	// Work from the base adrress of the main stack
	int8_t i;
	for(i = MAX_TASKS - 1; i >= 0; i--){
		if(i == MAX_TASKS - 1){
			tcb[i].stackPtr = (uint32_t *)(*MSP_BASE - 2000);
		}
		else{
			tcb[i].stackPtr = (uint32_t *)(*(tcb[i+1].stackPtr) - 1000);
		}
		// Init readyQueue
		tcbQueueInit(&readyQueues[i]);
	} 
	
	// Copy main stack to process stack of new main() task
	uint32_t stackSize = __get_MSP() - *MSP_BASE;
	uint32_t j;
	uint32_t *mainPtr;
	
	for(j = 0; j <= stackSize; j++){
		tcb[MAIN_TASK].stackPtr = (uint32_t *)(*(tcb[MAIN_TASK].stackPtr) - j);
		mainPtr = (uint32_t *)(*MSP_BASE - j); // Getting content from main stack
		*(tcb[MAIN_TASK].stackPtr) = *mainPtr; //
	}
	
	// Set MSP to the Main stack base address
	__set_MSP(*MSP_BASE);
	
	// Switch from MSP to PSP
	__set_CONTROL(__get_CONTROL() | (0x1 << 1));
	
	// Set PSP to address of the top of the stack of the main task
	__set_PSP((uint32_t) tcb[MAIN_TASK].stackPtr);
	
	// Update task count, set main task priority & current task index
	taskCount += 1;
	tcb[MAIN_TASK].priority = DEFAULT_PRIORITY;
	currTCB = &tcb[MAIN_TASK];
	
	// Configure Systick
	SysTick_Config(SystemCoreClock/1000);
}

// Create Task/Thread
bool rtosCreateTask(rtosTaskFunc_t func, void* argument, uint8_t priority){
	// Determine if there is an available TCB for the new task
	// Return false if there are no tasks available
	if (taskCount == MAX_TASKS){
		return false;
	}
	
	// Set priority of new task
	if (priority >= NUM_PRIORITY){
		priority = DEFAULT_PRIORITY;
	}
	tcb[taskCount].priority = priority;
	
	// Set state to READY. Enqueue task into readyQueue
	tcb[taskCount].state = READY;
	tcbQueueEnqueue(&readyQueues[priority], &tcb[taskCount]);
	non_empty_queue_bit_vector |= (1 << tcb[taskCount].priority);
	
	// Intialize stack of the task
	// Set PSR
	tcb[taskCount].stackPtr = (uint32_t *)(*tcb[taskCount].stackPtr - 1);
	*tcb[taskCount].stackPtr = 0x01000000;
	
	// Set PC
	tcb[taskCount].stackPtr = (uint32_t *)(*tcb[taskCount].stackPtr - 1);
	*tcb[taskCount].stackPtr = (uint32_t) func;
	
	// Set argument to task function
	tcb[taskCount].stackPtr = (uint32_t *)(*tcb[taskCount].stackPtr - 6);
	*tcb[taskCount].stackPtr = (uint32_t) argument;
	
	// Move stack pointer to R4 (may need to move SP too)
	tcb[taskCount].stackPtr = (uint32_t *)(*tcb[taskCount].stackPtr - 8);
	// __set_PSP(*tcb[taskCount].stackPtr);
	
	// Update task count
	taskCount += 1;
	
	return true;
}

// Conext switching
void PendSV_Handler(void){
	// Push reg contents onto the current task's stack
	// Record the current stack pointer in the current task's TCB
	*(currTCB->stackPtr) = storeContext();
	
	// Load the next task's top of stack address into the stack pointer
	// Pop register contents from the next task's stack
	restoreContext(*(nextTCB->stackPtr));
	
	// Set current TCB to next TCB
	currTCB = nextTCB;
}

// Scheduler
void SysTick_Handler(void) {
	// msTicks++;
	
	// Run scheduler if there are more than one task
	if(taskCount > 1){
		// Enqueue running task to back of its priority queue
		if(currTCB->state == RUNNING){
			tcbQueueEnqueue(&readyQueues[currTCB->priority], currTCB);
			non_empty_queue_bit_vector |= (1 << currTCB->priority);
			
			// Dequeue next task from priority queue to run
			uint8_t topPriority = 31 - __CLZ(non_empty_queue_bit_vector); // Get index of next non-empty queue
			nextTCB = tcbQueueDequeue(&readyQueues[topPriority]);
			if(tcbQueueEmpty(&readyQueues[topPriority])){ 								// set bit to 0 if the queue is empty
				non_empty_queue_bit_vector &= ~(1 << topPriority);
			}
			
			/* Call PendSV_Handler to perform the context switch
			 * This is done by setting the PendSV to pending. Pending bit should 
			 * automatically clear after interrupt is done */
			SCB->ICSR |= 1 << 28;
		}
	}
}
