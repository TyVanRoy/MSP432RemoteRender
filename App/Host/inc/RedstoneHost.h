/*
 * RedstoneHost.h
 *
 *  Created on: Apr 24, 2020
 *      Author: tyvanroy
 */

#ifndef APP_INC_REDSTONEHOST_H_
#define APP_INC_REDSTONEHOST_H_

#include "G8RTOS_Semaphores.h"

void InitRedstoneHost(void);
void LaunchRedstoneHost(void);

/* Threads */
void WaitForClient(void);
void SendToClient(void);
void ReceiveFromClient(void);

#endif /* APP_INC_REDSTONEHOST_H_ */
