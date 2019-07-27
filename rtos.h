/*
 * rtos header file implementation.
 * @author Steven Ha, Benjamin Duo 2019
 */

#ifndef __RTOS_H
#define __RTOS_H

#include <stdbool.h>
#include <stdint.h>
#include "tcb_queue.h"

#define MAX_TASKS 6
#define MAX_PRIORITY (MAX_TASKS - 1)
#define DEFAULT_PRIORITY 2
#define MAIN_TASK 0

extern uint32_t msTicks;

typedef void (*rtosTaskFunc_t)(void *args);

//Define Semaphore
typedef struct{
    int32_t count;
    tcb_queue_t waitList;
}osSem_Id;

//Define Mutex
typedef struct {
    bool mutex;
    tcb_queue_t waitList[MAX_TASKS];		// Waitlist for blocked tasks
		uint8_t waitListBitVector;
    tcb_t *owner;												// Pointer to owner of the task
    uint8_t ownerPriority;							// Original Priority of Owner
    //uint8_t h_prior; //Keep track of highest priority being blocked
}osMutex;

//define osStatus
typedef bool osStatus;
#define osOK 1
#define osError 0


// Initialize RTOS
void osInit(void);

// Starrt RTOS
void osKernalStart(void);

// Create Task
bool osCreateTask(rtosTaskFunc_t func, void* args, uint8_t priority);

// Task yields to next task
void osTaskYield(void);

// Delay a task for n milliseconds
void osDelay(uint32_t delay);

// SysTick Handler (Calls scheduler after time slice)
void SysTick_Handler(void);

// PendSV Handler (Scheduler, Performs Context Switching)
void PendSV_Handler(void);

// Idle Task
void idleTask(void *args);

// Initialize Semaphore
void init_Sema(osSem_Id *s, int32_t n);

// Wait for Semaphore
void osSemWait(osSem_Id *s);

// Signal for semaphore
void osSemSignal(osSem_Id *s);

// Intialize Mutex
void init_Mutex(osMutex *m);

// Acquire Mutex
void osAcquireMutex(osMutex *m);

// Release Mutex
void osReleaseMutex(osMutex *m);

// Dequeue in Mutex and Semaphore WaitList
osStatus tcbMutexDequeue(tcb_queue_t *queue, tcb_t* task);

// Save function status
void osEnterFunc(void);

// Restore function status
void osExitFunc(void);

//Timer
void osTimer(uint32_t delay);

// Print the priority of the current task
void osGetPriority(void);

#endif
