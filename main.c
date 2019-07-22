/*
 * Default main.c for rtos lab.
 * @author Andrew Morton, 2018
 */
#include <LPC17xx.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "uart.h"
#include "rtos.h"

//uint32_t msTicks = 0;

//void SysTick_Handler(void) {
//    msTicks++;
//}
void task1(void *args){
	printf("Task 2 running\n");
}

int main(void) {
	/*
	SysTick_Config(SystemCoreClock/1000);
	printf("\nStarting...\n\n");
	
	uint32_t period = 1000; // 1s
	uint32_t prev = -period;
	while(true) {
		if((uint32_t)(msTicks - prev) >= period) {
			printf("tick ");
			prev += period;
		}
	}
	*/
	printf("Init RTOS\n");
	rtosInit();
	bool status = rtosCreateTask(task1, NULL, 3);
	printf("Create Task Status: %d\n", status);
	while(1){
	}
}
