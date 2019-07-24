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

//void SysTick_Handler(void) {
//    msTicks++;
//}
void task1(void *args){
	//printf("Task 1 running\n");
	LPC_GPIO1->FIODIR |= 0xB0000000;
	LPC_GPIO2->FIODIR |= 0x0000007C;
	
	LPC_GPIO1->FIOCLR |= 0xB0000000;
	LPC_GPIO2->FIOCLR |= 0x0000007C;
	
	uint8_t num = 10;
	
	uint32_t gpio1_set = ((num & 0x3) | ((num & 0x4) << 1)) << 28;
	uint32_t gpio2_set = (num & 0xF8)>>1;
	
	
	LPC_GPIO1->FIOSET |= gpio1_set;
	LPC_GPIO2->FIOSET |= gpio2_set;
	while(true){
	}
}

int main(void) {
/*
	SysTick_Config(SystemCoreClock/1000);
	printf("\nStarting...\n\n");
*/	
	uint32_t period = 1000; // 1s
	uint32_t prev = -period;
	uint32_t num = 16;
	uint8_t priority = 31 - __CLZ(num);
	
	printf("Init RTOS ");
	rtosInit();
	printf("Main task running %d, \n", priority);
	bool status = rtosCreateTask(task1, NULL, 3);
	printf("Create Task Status: %d\n", status);
	
	while(true) {
		if((uint32_t)(msTicks - prev) >= period) {
			printf("tick ");
			prev += period;
		}
	}
}
