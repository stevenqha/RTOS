/*
 * rtos implementation.
 * @author Steven Ha, Benjamin Duo 2019
 */
#include "rtos.h"



//disable interrupts

//enable interrupts

//define osWaitForever
#define osWaitForever  0xFFFFFFFFU

//Define Mutex
typedef struct {
    bool mutex = 0;
}osMutex;


//Create Mutex
void init_Mutex(osMutex *m)

//Obtain Mutex
void osAcquireMutex(void *arg, ){ //need something for time
	
};

//Release Mutex
void osReleaseMutex(void *arg){
	
};

//Define Semaphore
typedef struct{
    uint32_t count = 0;
    //TCB *waitList
}osSem_Id;

//Initialize Semaphore
void init_Sema(osSem_Id *s, uint32_t n){
    s->count = n;
}

//Wait Semaphore
void osSemWait(osSem_Id *s){
    //disable interrupts
    (s->count)--;

    if(s->count < 0)
    {
        //block task
        //enable int
    }
    else
    //enable int
}

//Signal Semaphore
void osSemSignal(osSem_Id *s){
    //disable inter
    (s->count)++;
    if(s->count <= 0)
    {
        //unblock task
    }
    //enable interrupts
}