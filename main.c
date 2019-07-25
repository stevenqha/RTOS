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
//#define CONTEXT_SWITCHING
#define RANDOM_TESTING


#ifdef RANDOM_TESTING
void task1(void *args){	
	uint32_t period = 1000; // 1s
	uint32_t prev1 = -500;
	
	while(true){
		//printf("1");
		osDelay(2000);
		/*
		if((uint32_t)(msTicks - prev1) >= period) {	
			printf("msTicks1: %d\n", msTicks);
			prev1 += period;
		}
		*/
	}
}

void task2(void *args){
	uint32_t period = 1000; // 1s
	uint32_t prev2 = 0;
	
	while(true){
		//printf("3");
		osDelay(3000);
	}
}

int main(void) {	
	uint32_t period = 1000; // 1s
	uint32_t prev = -period;
	
	printf("Init RTOS \n");
	osInit();
	printf("Main task running, \n");
	bool status = osCreateTask(task1, NULL, 3);
	status = osCreateTask(task2, NULL, 3);
	
	while(true) {
		//printf("2");
		osDelay(1000);
		/*
		if((uint32_t)(msTicks - prev) >= period) {
			printf(" msTicks0: %d ", msTicks);
			prev += period;
		}*/
	}
}

#endif

// Showing that context switching and FPP
#ifdef CONTEXT_SWITCHING
void led_Task(void *args){
	LPC_GPIO1->FIODIR |= 0xB0000000;
	LPC_GPIO2->FIODIR |= 0x0000007C;
	LPC_GPIO2->FIODIR &= 0xFFFFFBFF;
	
	LPC_GPIO1->FIOCLR |= 0xB0000000;
	LPC_GPIO2->FIOCLR |= 0x0000007C;;
	uint32_t button;
	bool state = false;
	while(true){
		button = LPC_GPIO2->FIOPIN;
		if(0 == (button & (0x01<<10))){
				state = !state;
		}
		if(state){
			LPC_GPIO2->FIOSET |= (0x01<<2);
		}
		else{
			LPC_GPIO2->FIOCLR |= (0x01<<2); 
		}
		osThreadYield();
	}
}

void lcd_Task(void *args){
	LPC_GPIO1->FIODIR &= 0xF86FFFFF;
	
	GLCD_Init();
	GLCD_Clear(Blue);
	GLCD_SetBackColor(Blue);
	GLCD_SetTextColor(White);
	
	uint32_t joystick; 
	char s[32];
	
	while(true){
		// printf("");
		joystick = LPC_GPIO1->FIOPIN;
		
		if(0 == (joystick & 0x04000000)){
			sprintf(s, "Left");
		}
		else if(0 == (joystick & 0x02000000)){
			sprintf(s, "Down");
		}
		else if(0 == (joystick & 0x01000000)){
			sprintf(s, "Right");
		}
		else if(0 == (joystick & 0x00800000)){
			sprintf(s, "Up");
		}
		else{
			sprintf(s, "No dir");
		}
		GLCD_ClearLn(4, 1);
		GLCD_DisplayString(4,4,1, (unsigned char*)s);
		if(0 == (joystick & 0x00100000)){
			sprintf(s, "Pressed.");
		}
		else{
			sprintf(s, "Not pressed.");	
		}
		GLCD_ClearLn(5, 1);
		GLCD_DisplayString(5,4,1, (unsigned char*)s);
		// GLCD_Clear(Blue);
		
		osThreadYield();
	}
}

void pot_Task(void *args){
	LPC_PINCON->PINSEL1 &= ~(0x03<<18);
	LPC_PINCON->PINSEL1 |= (0x01<<18);
	LPC_SC->PCONP |= (0x01<<12);
	LPC_ADC->ADCR = (1 << 2) |     // select AD0.2 pin          
									(4 << 8) |     // ADC clock is 25MHz/5          
									(1 << 21);     // enable ADC
	
	uint32_t adcRead;
	
	while(true){
		LPC_ADC->ADCR |= (0x01<<24);
		while(LPC_ADC->ADGDR & (0x1<<31));
		adcRead = (LPC_ADC->ADGDR & (0xFFF<<4)) >> 4;
		printf("ADC: %d\n", adcRead);
	
		osThreadYield();
	}
}

int main(void){
	osInit();
	osCreateTask(led_Task, NULL, 5);
	osCreateTask(lcd_Task, NULL, 5);
	osCreateTask(pot_Task, NULL, 5);
	
	// Main task will not run because it's a lower priority
	while(true){
		//printf("");
		printf("Main task running");
	}
}
#endif 

