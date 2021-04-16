/*
 * RedstoneClient.h
 *
 *  Created on: Apr 24, 2020
 *      Author: tyvanroy
 */

#ifndef APP_INC_REDSTONECLIENT_H_
#define APP_INC_REDSTONECLIENT_H_

#include "Redstone.h"
#include "G8RTOS_Semaphores.h"

struct ControlPacket_t ControlPacket;
semaphore_t CONTROLLOCK;

void InitRedstoneClient(void);
void LaunchRestoneClient(void);

/* Threads */
void ConnectToHost(void);


#endif /* APP_INC_REDSTONECLIENT_H_ */
