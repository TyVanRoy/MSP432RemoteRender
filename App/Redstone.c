/*
 * Redstone.c
 *
 *  Created on: Apr 24, 2020
 *      Author: tyvanroy
 */

#include "Redstone.h"
#include "demo_sysctl.h"
#include "MainMenu.h"

/*
 * Note to Professor: If you want to run this, you must use our G8RTOS, as it is modified to handle floating point operations.
 */


void InitRedstone(void){
    G8RTOS_Init();
    BSP_InitBoard(true);
    InitRedUI();
    InitRedLCD();

    G8RTOS_InitSemaphore(&WIFILOCK, 1);
}

void LaunchRedstone(void){
    InitMainMenu();
    G8RTOS_Launch();
}


