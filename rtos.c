/*
 * rtos implementation.
 * @author Steven Ha, Benjamin Duo 2019
 */
#include <LPC17xx.h>
#include "rtos.h"
#include "tcb_queue.h"

tcb_t tcb[MAX_TASKS];
const uint32_t *MSP_BASE = (uint32_t *)0x0;

schedule_t schedule;
tcb_queue_t readyQueues[NUM_PRIORITY];
uint32_t non_empty_queue_bit_vector = 0;
	
uint32_t msTicks = 0;
uint8_t taskCount = 0;

// Initialize RTOS
void rtosInit(){		
	// Intialize each TCB with the base address of each stack
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
		mainPtr = (uint32_t *)(*MSP_BASE - j);
		*(tcb[MAIN_TASK].stackPtr) = *mainPtr;
	}
	
	// Set MSP to the Main stack base address
	__set_MSP(*MSP_BASE);
	
	// Switch from MSP to PSP
	__set_CONTROL(0x2);
	
	// Set PSP to address of the top of the stack of the main task
	__set_PSP((uint32_t) tcb[MAIN_TASK].stackPtr);
	
	// Update task count, set main task priority & current task index
	taskCount += 1;
	tcb[MAIN_TASK].priority = DEFAULT_PRIORITY;
	schedule.curr = MAIN_TASK;
	
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
	// Move stack pointer to R4
	tcb[taskCount].stackPtr = (uint32_t *)(*tcb[taskCount].stackPtr - 8);
	
	// Update task count
	taskCount += 1;
	
	return true;
}

// Conext switching
void PendSV_Handler(void){
	// Push reg contents onto the current task's stack
	// Record the current stack pointer in the current task's TCB
	*(tcb[schedule.curr].stackPtr) = storeContext();
	
	// Load the next task's top of stack address into the stack pointer
	// Pop register contents from the next task's stack
	restoreContext(*(tcb[schedule.next].stackPtr));
	
	// Set current task index to next index
	schedule.curr = schedule.next;
}

// Scheduler
void SysTick_Handler(void) {
	msTicks++;
	
	// Run scheduler if there are more than one task
	if(taskCount > 1){
		// Enqueue running task to back of its priority queue
		if(tcb[schedule.curr].state == RUNNING){
			tcbQueueEnqueue(&readyQueues[tcb[schedule.curr].priority], &tcb[schedule.curr]);
			non_empty_queue_bit_vector |= (1 << tcb[schedule.curr].priority);
			
			// Dequeue next task from priority queue to run
			
			/* Call PendSV_Handler to perform the context switch
			 * This is done by setting the PendSV to pending. Pending bit should 
			 * automatically clear after interrupt is done */
			SCB->ICSR |= 1 << 28;
		}

	}
}
