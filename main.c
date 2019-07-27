/*
 * Default main.c for rtos lab.
 * @author Andrew Morton, 2018
 */
#include <LPC17xx.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "uart.h"
#include "GLCD.h"
#include "rtos.h"

// Choose which test to run
#define CONTEXT_SWITCHING
//#define SEMA
//#define MUTEX
//#define PRIORITY_INHERITANCE


/***************************************************************************/
/* Conext Switching and FPP Test Case. Working                             */
#ifdef CONTEXT_SWITCHING
void task1(void *args){
	while(true){
		printf("Task 1 ");
		osDelay(1000);
	}
}

void task2(void *args){
	osDelay(500);
	while(true){
		printf("Task 2\n");
		osDelay(1000);
	}
}

int main(void) {	
	printf("Init RTOS \n");
	osInit();

	osCreateTask(task1, NULL, 3);
	osCreateTask(task2, NULL, 3);
	osKernalStart();
	
	while(true) {
		printf("Main\n");
		osDelay(1000);
	}
}

#endif


/***************************************************************************/
/* Blocking semaphore test case. Working.                                  */
#ifdef SEMA
osSem_Id s1;
uint32_t count = 0;

void task1(void *args){ //prior 3
    uint32_t a = 0;
	  while(true){
			printf("a: %d\n", a);
			a++;
			osSemSignal(&s1);
			printf("sem count: %d\n", s1.count);
    }
}

void task2(void *args){ //prior 5
    uint32_t b = 0;
	  while(true){
			printf("b: %d\n", b);
			b++;
			osSemWait(&s1);
    }
}

int main(void){
	printf("Start ");

	//intialize RTOS
	osInit();
	init_Sema(&s1, 2);

	osCreateTask(task1, NULL, 3);
	osCreateTask(task2, NULL, 5);
	printf("Tasks Created\n");
	osKernalStart();

	// Main task will not run because it's a lower priority
	while(true){
		printf("Main task running");
		osDelay(1000);
	}
	
}
#endif


/***************************************************************************/
/* Mutex Owner Test on Release Test Case. Working                          */
#ifdef MUTEX
osMutex m1;
uint32_t count = 0;

void task1(void *args){ //prior 4
	osDelay(100);
	while(true){
		osAcquireMutex(&m1);
		printf("Task 1\n");
		//printf("This is a very long message that should run aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaasss\n");
		osReleaseMutex(&m1);
		osTaskYield();
	}
}

int main(void){
	printf("Mutex test\n");

	//intialize RTOS
	osInit();
	init_Mutex(&m1);
	osCreateTask(task1, NULL, 2);
	printf("Tasks Created\n");
	osKernalStart();

	// Main task will not run because it's a lower priority
	while(true){
		osAcquireMutex(&m1);
		printf("Main task\n");
		osDelay(1000);	
		osReleaseMutex(&m1);
		osReleaseMutex(&m1);
		osTaskYield();
	}
}
#endif


/***************************************************************************/
/* Priority Inheritance Test Case     :[                                   */
#ifdef PRIORITY_INHERITANCE
osMutex m1;
osSem_Id s1;

uint32_t count = 0;

void task1(void *args){ //prior 4
	printf("Task 1 waiting on task2\n");
	osSemWait(&s1);		// Task 1 waits until main signals s1
	printf("Task 1 Synced\n");
	osAcquireMutex(&m1);
	printf("Task 1\n");
	osReleaseMutex(&m1);
	osTaskYield();
	
	while(true){
	}
}

void task2(void *args){
	osAcquireMutex(&m1);		
	osSemSignal(&s1);
	printf("released semaphore\n");
	osDelay(1000);
	osGetPriority();	// Priority changes to 4
	osReleaseMutex(&m1);
	osGetPriority();	// Priority changes back to 2
	osTaskYield();
	
	while(true){		
	}
}

int main(void){
	printf("Mutex test\n");

	//intialize RTOS
	osInit();
	init_Sema(&s1, 0);
	init_Mutex(&m1);
	osCreateTask(task1, NULL, 4);
	osCreateTask(task2, NULL, 3);
	printf("Tasks Created\n");
	osKernalStart();
	
	// Main task will not run because it's a lower priority
	while(true){
		osDelay(2000);
		printf("Main is running\n");
		osTaskYield();
	}
}
#endif
