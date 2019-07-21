/*
 * rtos header file implementation.
 * @author Steven Ha, Benjamin Duo 2019
 */

#ifndef RTOS
#define RTOS

#include <stdbool.h>
#include <stdint.h>
#include "context.h"

#define MAX_TASKS 6
#define NUM_PRIORITY (MAX_TASKS - 1)
#define DEFAULT_PRIORITY 3
#define MAIN_TASK 0

typedef void (*rtosTaskFunc_t)(void *args);

enum state_t{READY, RUNNING, BLOCKED, WAITING, INACTIVE};

// TCB Control Block Struct
typedef struct tcb{
	uint8_t id;
	uint8_t priority;
	struct tcb *p_next_tcb;	
	enum state_t state;
	uint32_t *stackPtr;
} tcb_t;

// Scheduler Control Block Struct
typedef struct{
	uint8_t curr;		// Index of current task 
	uint8_t next; 	// Index of next task
	tcb_t *running; // TCB of running task
} schedule_t;

// Semaphore Struct
typedef struct{

}sem_t;
// Mutex Struct
typedef struct{

}mutex_t;

// Initialize RTOS
void rtosInit(void);

// Create Task/Thread
bool rtosCreateTask(rtosTaskFunc_t func, void* argument, uint8_t priority);

// PendSV Handler (For context switching)
void PendSV_Handler(void);

// SysTick Handler (Scheduler)
void SysTick_Handler(void);
	
// Initialize Semaphore
void semInit(sem_t *s, uint32_t count, uint32_t maxCount);

// Wait for Semaphore
void semWait(sem_t *s);

// Signal for semaphore
void semSignal(sem_t *s);

// Intialize Mutex
void mutexInit(mutex_t *m, uint8_t count);

// Wait for Mutex
void mutexWait(mutex_t *s);

// Signal for Mutex
void mutexSignal(mutex_t *s);

#endif
