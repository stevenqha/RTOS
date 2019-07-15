/*
 * rtos header file implementation.
 * @author Steven Ha, Benjamin Duo 2019
 */

#ifndef RTOS
#define RTOS

#include <stdbool.h>
#include <stdint.h>

enum state_t{ready, running, blocked, waiting, inactive, terminated};

// TCB Control Block Struct
typedef struct{
	uint8_t id;
	uint8_t priority;	
	enum state_t state;
}tcb_t;

// Semaphore Struct
typedef struct{

}sem_t;
// Mutex Struct
typedef struct{

}mutex_t;

// Initialize RTOS
void osInit();

// Create Task/Thread
void osCreateTask();

// Initializa Semaphore
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