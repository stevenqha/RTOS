/*
 * rtos implementation.
 * @author Steven Ha, Benjamin Duo 2019
 */
 #include "rtos.h"



//disable interrupts

//enable interrupts


//Create Mutex


//Obtain Mutex
void osAcquireMutex(void *arg){
	
};

//Release Mutex
void osReleaseMutex(void *arg){
	
};

//Define Semaphore
typedef uint32_t osSem_Id;

//Initialize Semaphore
void init_Sema(osSem_Id *s, uint32_t n)
{
    &s = n;
}


//Wait Semaphore
void osSemWait(osSem_Id *s)
{
    //disable interrupts
    while(*s =< 0)
    {
        //enable int
        //disable int
    }
    (*s)--;
    //enable int
}

//Signal Semaphore
void osSemSignal(osSem_Id *s)
{
    //disable inter
    (s*)++;
    //enable inter
}