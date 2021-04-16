/*
 * BSP.c
 *
 *  Created on: Dec 30, 2016
 *      Author: Raz Aloni
 */

#include <driverlib.h>
#include "BSP.h"
#include "i2c_driver.h"
#include "LCDLib.h"

/* Initializes the entire board */
void BSP_InitBoard()
{
	/* Disable Watchdog */
	WDT_A_clearTimer();
	WDT_A_holdTimer();

    /* Enable FPU */
	SCB->CPACR |= ((3UL << 10 * 2) |                       // Set CP10 Full Access
                   (3UL << 11 * 2));                       // Set CP11 Full Access

	/* Initialize Clock */
	ClockSys_SetMaxFreq();

	/* Init i2c */
	initI2C();

	/* Init Opt3001 */
	sensorOpt3001Enable(true);

	/* Init Tmp007 */
	sensorTmp007Enable(true);

	/* Init Bmi160 */
    bmi160_initialize_sensor();

    /* Init joystick without interrupts */
	Joystick_Init_Without_Interrupt();

	/* Init Bme280 */
	bme280_initialize_sensor();

	/* Init BackChannel UART */
	BackChannelInit();
	//uartInit();

	/* Init RGB LEDs */
	init_RGBMod();

	/* Init LCD */
    __disable_interrupts();
	LCD_Init(1);
    __disable_interrupts();
}


