/*
 * RedLib.h
 *
 *  Created on: Mar 3, 2020
 *      Author: tyvanroy
 */

#ifndef REDLIB_REDLIB_H_
#define REDLIB_REDLIB_H_

#include <G8RTOS/G8RTOS.h>
#include "BSP.h"
#include "msp.h"
#include "stdio.h"
#include "stdbool.h"
#include "stdlib.h"
#include "math.h"
#include "driverlib.h"
#include "BackChannelUart.h"
#include "RedLCD.h"
#include "Red3D.h"
#include "RedUI.h"
#include "RedTypes.h"

/**
 *
 * Call initialization functions before using each module.
 *
 * Call InitRedLCD before using RedUI !
 *
 * Call G8RTOS_Init before initializing RedUI !
 *
 **/

/** Newton's Square Root **/
int NewtRoot(int x);

/** UART **/
inline void TimeDebug(char* tag, int i);
inline void Debug(char* tag, int i);
inline void Print(char* string);
inline void PrintLn(char* string);
inline void PrintInt(int i);
inline void PrintFloat(float f);

#endif /* REDLIB_REDLIB_H_ */
