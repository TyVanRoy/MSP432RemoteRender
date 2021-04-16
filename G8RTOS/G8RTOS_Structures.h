/*
 * G8RTOS_Structure.h
 *
 *  Created on: Jan 12, 2017
 *      Author: Raz Aloni
 */

#ifndef G8RTOS_STRUCTURES_H_
#define G8RTOS_STRUCTURES_H_

#include "../G8RTOS/G8RTOS.h"
#include "../G8RTOS/G8RTOS_Semaphores.h"
#include "../G8RTOS/G8RTOS_Scheduler.h"

#define FIFOSIZE 16
#define MAX_NAME_LENGTH 16

typedef int32_t semaphore_t;

typedef uint32_t threadId_t;

/*********************************************** Data Structure Definitions ***********************************************************/

/*
 *  Thread Control Block:
 *      - Every thread has a Thread Control Block
 *      - The Thread Control Block holds information about the Thread Such as the Stack Pointer, Priority Level, and Blocked Status
 *      - For Lab 2 the TCB will only hold the Stack Pointer, next TCB and the previous TCB (for Round Robin Scheduling)
 */

typedef struct tcb_t{
    int32_t* tsp;
    struct tcb_t* nextTCB;
    struct tcb_t* prevTCB;
    semaphore_t* blocked;
    uint8_t Priority;
    uint32_t SleepCount;
    bool Alive;
    bool Asleep;
    threadId_t threadID;
    char threadName[MAX_NAME_LENGTH];
} tcb_t;

/*
 *  Periodic Thread Control Block:
 *      - Holds a function pointer that points to the periodic thread to be executed
 *      - Has a period in us
 *      - Holds Current time
 *      - Contains pointer to the next periodic event - linked list
 */

typedef struct ptcb_t{
    void(*Handler)(void);
    uint32_t Period;
    uint32_t ExecuteTime;
    uint32_t CurrentTime;
    struct ptcb_t* nextEvent;
    struct ptcb_t* prevEvent;
} ptcb_t;

/*********************************************** Data Structure Definitions ***********************************************************/


/*********************************************** Public Variables *********************************************************************/

tcb_t * CurrentlyRunningThread;

/*********************************************** Public Variables *********************************************************************/




#endif /* G8RTOS_STRUCTURES_H_ */
