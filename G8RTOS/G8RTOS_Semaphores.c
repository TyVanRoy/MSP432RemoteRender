/*
 * G8RTOS_Semaphores.c
 */

/*********************************************** Dependencies and Externs *************************************************************/

#include <G8RTOS/G8RTOS_CriticalSection.h>
#include <G8RTOS/G8RTOS_Semaphores.h>
#include <stdint.h>
#include "msp.h"

/*********************************************** Dependencies and Externs *************************************************************/


/*********************************************** Public Functions *********************************************************************/

/*
 * Initializes a semaphore to a given value
 * Param "s": Pointer to semaphore
 * Param "value": Value to initialize semaphore to
 * THIS IS A CRITICAL SECTION
 */
void G8RTOS_InitSemaphore(semaphore_t *s, int32_t value)
{
    int status = StartCriticalSection();
    *s = value;
    EndCriticalSection(status);
}

/*
 * No longer waits for semaphore
 *  - Decrements semaphore
 *  - Blocks thread is sempahore is unavalible
 * Param "s": Pointer to semaphore to wait on
 * THIS IS A CRITICAL SECTION
 */
void SemWait(semaphore_t *s)
{
    int status = StartCriticalSection();
    (*s)--;

    if((*s) < 0){
        CurrentlyRunningThread->blocked = s;
        //EndCriticalSection(status);
        StartContextSwitch();
    }

    EndCriticalSection(status);
}

/*
 * Signals the completion of the usage of a semaphore
 *  - Increments the semaphore value by 1
 *  - Unblocks any threads waiting on that semaphore
 * Param "s": Pointer to semaphore to be signaled
 * THIS IS A CRITICAL SECTION
 */
void SemSignal(semaphore_t *s)
{
    int status = StartCriticalSection();

    (*s)++;

    if((*s) <= 0){
        tcb_t* t = CurrentlyRunningThread->nextTCB;

        while(t->blocked != s){
            t = t->nextTCB;
        }

        t->blocked = 0;
    }

    EndCriticalSection(status);
}

/*********************************************** Public Functions *********************************************************************/


