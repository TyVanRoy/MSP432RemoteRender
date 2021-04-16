/*
 * G8RTOS_Scheduler.c
 */

/*********************************************** Dependencies and Externs *************************************************************/

#include <G8RTOS/G8RTOS_Scheduler.h>
#include <G8RTOS/G8RTOS_Semaphores.h>
#include <G8RTOS/G8RTOS_CriticalSection.h>
#include <stdint.h>
#include <string.h>
#include "msp.h"

/*
 * G8RTOS_Start exists in asm
 */
extern void G8RTOS_Start();

/* System Core Clock From system_msp432p401r.c */
extern uint32_t SystemCoreClock;

/*********************************************** Dependencies and Externs *************************************************************/


/*********************************************** Defines ******************************************************************************/

/* Status Register with the Thumb-bit Set */
#define THUMBBIT 0x01000000
#define PENDSVSET 0x00000001 << 28
#define FLOAT_CONTEXT_SIZE 18

/*********************************************** Defines ******************************************************************************/


/*********************************************** Data Structures Used *****************************************************************/

/* Thread Control Blocks
 *	- An array of thread control blocks to hold pertinent information for each thread
 */
static tcb_t threadControlBlocks[MAX_THREADS];
static uint16_t IDCounter = 0;

/* Thread Stacks
 *	- An array of arrays that will act as invdividual stacks for each thread
 */
static int32_t threadStacks[MAX_THREADS][STACKSIZE];

/* Periodic Event Threads
 * - An array of periodic events to hold pertinent information for each thread
 */
static ptcb_t Pthreads[MAX_PTHREADS];

/*********************************************** Data Structures Used *****************************************************************/


/*********************************************** Private Variables ********************************************************************/

/*
 * Current Number of Threads currently in the scheduler
 */
static uint32_t NumberOfThreads;

/*
 * Current Number of Periodic Threads currently in the scheduler
 */
static uint32_t NumberOfPthreads;

/*********************************************** Private Variables ********************************************************************/


/*********************************************** Private Functions ********************************************************************/

/*
 * Initializes the Systick and Systick Interrupt
 * The Systick interrupt will be responsible for starting a context switch between threads
 * Param "numCycles": Number of cycles for each systick interrupt
 */
static void InitSysTick(uint32_t numCycles)
{
    SysTick_Config(numCycles);
    SysTick_enableInterrupt();
}

/*
 * Chooses the next thread to run.
 * Lab 2 Scheduling Algorithm:
 * 	- Simple Round Robin: Choose the next running thread by selecting the currently running thread's next pointer
 * 	- Check for sleeping and blocked threads
 */
void G8RTOS_Scheduler()
{
    uint8_t nextThreadPriority = UINT8_MAX;
    tcb_t* nextTemp = CurrentlyRunningThread->nextTCB;

    for(int i = 0; i < NumberOfThreads + 1; i++){
        if(!nextTemp->blocked && !nextTemp->Asleep){
            if(nextTemp->Priority < nextThreadPriority){
                CurrentlyRunningThread = nextTemp;
                nextThreadPriority = CurrentlyRunningThread->Priority;
            }
        }
        nextTemp = nextTemp->nextTCB;
    }
}


/*
 * SysTick Handler
 * The Systick Handler now will increment the system time,
 * set the PendSV flag to start the scheduler,
 * and be responsible for handling sleeping and periodic threads
 */
void SysTick_Handler()
{
    tcb_t* t = CurrentlyRunningThread;
    for(int i = 0; i < NumberOfThreads; i++){
        if(t->Asleep){
            if(t->SleepCount <= SystemTime){
                t->Asleep = false;
                t->SleepCount = 0;
            }
        }

        t = t->nextTCB;
    }

    ptcb_t* event = &Pthreads[0];
    for(int i = 0; i < NumberOfPthreads; i++){
        if(event->ExecuteTime == SystemTime){
            event->ExecuteTime = event->Period + SystemTime;
            Pthreads[i].Handler();
        }
        event = event->nextEvent;
    }

    SystemTime++;
    StartContextSwitch();
}

/*********************************************** Private Functions ********************************************************************/


/*********************************************** Public Variables *********************************************************************/

/* Holds the current time for the whole System */
uint32_t SystemTime;

/*********************************************** Public Variables *********************************************************************/


/*********************************************** Public Functions *********************************************************************/

/*
 * Sets variables to an initial state (system time and number of threads)
 * Enables board for highest speed clock and disables watchdog
 */
void G8RTOS_Init()
{
    for(int i = 0; i < MAX_THREADS; i++){
        threadControlBlocks[i].Alive = false;
    }

    // Relocate vector table to SRAM for aperiodic events
    uint32_t newVTORTable = 0x20000000;
    memcpy((uint32_t *)newVTORTable, (uint32_t *)SCB->VTOR, 57*4); // 57 interrupt vectors to copy
    SCB->VTOR = newVTORTable;

    SystemTime = 0;                             // System time
    NumberOfThreads = 0;
    NumberOfPthreads = 0;
}

void Idle(void){
    while(1);
}

/*
 * Starts G8RTOS Scheduler
 * 	- Initializes the Systick
 * 	- Sets Context to first thread
 * Returns: Error Code for starting scheduler. This will only return if the scheduler fails
 */
int G8RTOS_Launch()
{
    G8RTOS_AddThread(Idle, 0xfe, "idle");

    __disable_interrupts();

    G8RTOS_Scheduler();

   NVIC_SetPriority (PendSV_IRQn, 0xff);

   InitSysTick(ClockSys_GetSysFreq() / 1000);

   __disable_interrupts();

   NVIC_SetPriority (SysTick_IRQn, 0xfe);

   G8RTOS_Start();

   return 1;
}


/*
 * Adds threads to G8RTOS Scheduler
 * 	- Checks if there are stil available threads to insert to scheduler
 * 	- Initializes the thread control block for the provided thread
 * 	- Initializes the stack for the provided thread to hold a "fake context"
 * 	- Sets stack tcb stack pointer to top of thread stack
 * 	- Sets up the next and previous tcb pointers in a round robin fashion
 * Param "threadToAdd": Void-Void Function to add as preemptable main thread
 * Returns: Error code for adding threads
 */
sched_ErrCode_t G8RTOS_AddThread(void (*threadToAdd)(void), uint8_t priority, char* name){
    int32_t IBit_State = StartCriticalSection();

    // check availability
    if(NumberOfThreads >= MAX_THREADS){
        EndCriticalSection(IBit_State);
        return THREAD_LIMIT_REACHED;
    }

    // check for the first dead thread
    int pos = 0;
    while(threadControlBlocks[pos].Alive){
        pos++;
    }

    if(pos >= MAX_THREADS){
        EndCriticalSection(IBit_State);
        return THREADS_INCORRECTLY_ALIVE;
    }

    // if its not the first thread added, add it after the currently running thread
    if(NumberOfThreads == 0){
        threadControlBlocks[pos].nextTCB = &threadControlBlocks[pos];
        threadControlBlocks[pos].prevTCB = &threadControlBlocks[pos];

        CurrentlyRunningThread = &threadControlBlocks[pos];
    }else{
        threadControlBlocks[pos].prevTCB = CurrentlyRunningThread;
        threadControlBlocks[pos].nextTCB = CurrentlyRunningThread->nextTCB;

        CurrentlyRunningThread->nextTCB->prevTCB = &threadControlBlocks[pos];
        CurrentlyRunningThread->nextTCB = &threadControlBlocks[pos];
    }

    // initialize thread stack
    if(!(threadStacks[pos][STACKSIZE - 1 - FLOAT_CONTEXT_SIZE] & THUMBBIT)){
        threadStacks[pos][STACKSIZE - 1 - FLOAT_CONTEXT_SIZE] = THUMBBIT;
    }
    threadStacks[pos][STACKSIZE - 2 - FLOAT_CONTEXT_SIZE] = (int32_t) threadToAdd;

    // initialize thread control block
    strcpy(threadControlBlocks[pos].threadName, name);
    threadControlBlocks[pos].tsp = &threadStacks[pos][STACKSIZE - 16 - FLOAT_CONTEXT_SIZE];
    threadControlBlocks[pos].threadID = ((IDCounter++) << 16) | pos;
    threadControlBlocks[pos].Priority = priority;
    threadControlBlocks[pos].Alive = true;
    threadControlBlocks[pos].Asleep = false;

    NumberOfThreads++;

    EndCriticalSection(IBit_State);
    return NO_ERROR;
}


/*
 * Adds periodic threads to G8RTOS Scheduler
 * Function will initialize a periodic event struct to represent event.
 * The struct will be added to a linked list of periodic events
 * Param Pthread To Add: void-void function for P thread handler
 * Param period: period of P thread to add
 * Returns: Error code for adding threads
 */
sched_ErrCode_t G8RTOS_AddPeriodicEvent(void (*PthreadToAdd)(void), uint32_t period)
{
    int32_t IBit_State = StartCriticalSection();

    if(NumberOfPthreads >= MAX_THREADS){
        EndCriticalSection(IBit_State);
        return THREAD_LIMIT_REACHED;
    }

    Pthreads[0].ExecuteTime = 0;

    if(NumberOfPthreads == 0){
        Pthreads[0].nextEvent = &Pthreads[0];
        Pthreads[0].prevEvent = &Pthreads[0];
    }else{
        Pthreads[NumberOfPthreads].nextEvent = &Pthreads[0];
        Pthreads[NumberOfPthreads].prevEvent = &Pthreads[NumberOfPthreads - 1];
        Pthreads[NumberOfPthreads - 1].nextEvent = &Pthreads[NumberOfPthreads];

        // only works for the first multiple found
        for(int i = 0; i < NumberOfPthreads; i++){
            if(Pthreads[NumberOfPthreads].Period / Pthreads[i].Period == 0){
                Pthreads[NumberOfPthreads].ExecuteTime++;
                break;
            }else if(Pthreads[i].Period / Pthreads[NumberOfPthreads].Period == 0){
                Pthreads[NumberOfPthreads - 1].ExecuteTime++;
            }
        }
    }


    Pthreads[NumberOfPthreads].Period = period;
    Pthreads[NumberOfPthreads].Handler = PthreadToAdd;

    NumberOfPthreads++;

    EndCriticalSection(IBit_State);
    return NO_ERROR;
}

sched_ErrCode_t G8RTOS_AddAPeriodicEvent(void (*AthreadToAdd)(void), uint8_t priority, IRQn_Type IRQn){
    if(IRQn < PSS_IRQn || IRQn > PORT6_IRQn){
        return IRQn_INVALID;
    }else if(priority >= OSINT_PRIORITY){
        return HWI_PRIORITY_INVALID;
    }

    __NVIC_SetVector(IRQn, (int32_t) AthreadToAdd);
    __NVIC_SetPriority(IRQn, priority);
    NVIC_EnableIRQ(IRQn);

    return NO_ERROR;
}

inline void StartContextSwitch(void){
    SCB->ICSR |= PENDSVSET;
}

/*
 * Puts the current thread into a sleep state.
 *  param durationMS: Duration of sleep time in ms
 */
void sleep(uint32_t durationMS)
{
    CurrentlyRunningThread->SleepCount = SystemTime + durationMS;
    CurrentlyRunningThread->Asleep = true;
    StartContextSwitch();
}

threadId_t G8RTOS_GetThreadId(void){
    return CurrentlyRunningThread->threadID;
}

sched_ErrCode_t G8RTOS_GetThreadIdByName(char* name, threadId_t* id){

    tcb_t* target = CurrentlyRunningThread;
    bool found = false;
    for(int i = 0; i < NumberOfThreads; i++){
        if(!strcmp(target->threadName, name)){
            found = true;
            break;
        }
        target = target->nextTCB;
    }

    if(!found){
        return THREAD_DOES_NOT_EXIST;
    }

    *id = target->threadID;
    return NO_ERROR;
}

sched_ErrCode_t G8RTOS_KillThread(threadId_t threadID){
    int32_t IBit_State = StartCriticalSection();

    // check if there is only 1 thread running
    if(NumberOfThreads == 1){
        EndCriticalSection(IBit_State);
        return CANNOT_KILL_LAST_THREAD;
    }

    // check if the target exists
    tcb_t* target = CurrentlyRunningThread;
    bool found = false;
    for(int i = 0; i < NumberOfThreads; i++){
        if(target->threadID == threadID){
            found = true;
            break;
        }
        target = target->nextTCB;
    }
    if(!found){
        EndCriticalSection(IBit_State);
        return THREAD_DOES_NOT_EXIST;
    }

    // kill the target
    target->Alive = false;

    tcb_t* targetPrev = target->prevTCB;
    targetPrev->nextTCB = target->nextTCB;
    target->nextTCB->prevTCB = targetPrev;


    NumberOfThreads--;

    if(target == CurrentlyRunningThread){
        EndCriticalSection(IBit_State);
        StartContextSwitch();
    }else{
        EndCriticalSection(IBit_State);
    }
    return NO_ERROR;
}

sched_ErrCode_t G8RTOS_KillSelf(void){
    int32_t IBit_State = StartCriticalSection();

    // check if there is only 1 thread running
    if(NumberOfThreads == 1){
        EndCriticalSection(IBit_State);
        return CANNOT_KILL_LAST_THREAD;
    }

    // kill the target
    CurrentlyRunningThread->Alive = false;

    tcb_t* prev = CurrentlyRunningThread->prevTCB;
    prev->nextTCB = CurrentlyRunningThread->nextTCB;
    CurrentlyRunningThread->nextTCB->prevTCB = prev;

    NumberOfThreads--;

    EndCriticalSection(IBit_State);

    StartContextSwitch();
    return NO_ERROR;
}


/*********************************************** Public Functions *********************************************************************/
