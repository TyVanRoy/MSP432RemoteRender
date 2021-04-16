/*
 * G8RTOS_IPC.c
 *
 *  Created on: Jan 10, 2017
 *      Author: Daniel Gonzalez
 */
#include <G8RTOS/G8RTOS_IPC.h>
#include <G8RTOS/G8RTOS_Semaphores.h>
#include <G8RTOS/G8RTOS_Structures.h>
#include <stdint.h>
#include "msp.h"

/*********************************************** Defines ******************************************************************************/

#define MAX_NUMBER_OF_FIFOS 4

/*********************************************** Defines ******************************************************************************/


/*********************************************** Data Structures Used *****************************************************************/


/*
 * FIFO struct will hold
 *  - buffer
 *  - head
 *  - tail
 *  - lost data
 *  - current size
 *  - mutex
 */

typedef struct{
    int32_t Buffer[FIFOSIZE];
    int32_t* Head;
    int32_t* Tail;
    uint32_t LostData;
    semaphore_t CurrentSize;
    semaphore_t Mutex;
} FIFO_t;

static FIFO_t FIFOs[4];

/*********************************************** Data Structures Used *****************************************************************/

/*
 * Initializes FIFO Struct
 */
int G8RTOS_InitFIFO(uint32_t FIFOIndex)
{
    if(FIFOIndex >=4){
        return 1;
    }

    FIFOs[FIFOIndex].Head = &FIFOs[FIFOIndex].Buffer[0];
    FIFOs[FIFOIndex].Tail = &FIFOs[FIFOIndex].Buffer[0];
    FIFOs[FIFOIndex].LostData = 0;
    G8RTOS_InitSemaphore(&FIFOs[FIFOIndex].CurrentSize, 0);
    G8RTOS_InitSemaphore(&FIFOs[FIFOIndex].Mutex, 1);

    return 0;
}

/*
 * Reads FIFO
 *  - Waits until CurrentSize semaphore is greater than zero
 *  - Gets data and increments the head ptr (wraps if necessary)
 * Param: "FIFOChoice": chooses which buffer we want to read from
 * Returns: uint32_t Data from FIFO
 */
uint32_t readFIFO(uint32_t FIFOChoice)
{
    SemWait(&FIFOs[FIFOChoice].CurrentSize);
    SemWait(&FIFOs[FIFOChoice].Mutex);

    int value = *FIFOs[FIFOChoice].Head;

    if(FIFOs[FIFOChoice].Head == &FIFOs[FIFOChoice].Buffer[FIFOSIZE - 1]){
        FIFOs[FIFOChoice].Head = &FIFOs[FIFOChoice].Buffer[0];
    }else{
        FIFOs[FIFOChoice].Head++;
    }

    SemSignal(&FIFOs[FIFOChoice].Mutex);

    return value;
}

/*
 * Writes to FIFO
 *  Writes data to Tail of the buffer if the buffer is not full
 *  Increments tail (wraps if ncessary)
 *  Param "FIFOChoice": chooses which buffer we want to read from
 *        "Data': Data being put into FIFO
 *  Returns: error code for full buffer if unable to write
 */
int writeFIFO(uint32_t FIFOChoice, uint32_t Data)
{
    if(FIFOs[FIFOChoice].CurrentSize >= FIFOSIZE){
        // overwrite old data
        *FIFOs[FIFOChoice].Head = Data;
        FIFOs[FIFOChoice].Tail = FIFOs[FIFOChoice].Head;

        if(FIFOs[FIFOChoice].Head == &FIFOs[FIFOChoice].Buffer[FIFOSIZE - 1]){
            FIFOs[FIFOChoice].Head = &FIFOs[FIFOChoice].Buffer[0];
        }else{
            FIFOs[FIFOChoice].Head++;
        }

        FIFOs[FIFOChoice].LostData++;

        return 1;
    }

    *FIFOs[FIFOChoice].Tail = Data;

    if(FIFOs[FIFOChoice].Tail == &FIFOs[FIFOChoice].Buffer[FIFOSIZE - 1]){
        FIFOs[FIFOChoice].Tail = &FIFOs[FIFOChoice].Buffer[0];
    }else{
        FIFOs[FIFOChoice].Tail++;
    }

    SemSignal(&FIFOs[FIFOChoice].CurrentSize);
    return 0;
}

