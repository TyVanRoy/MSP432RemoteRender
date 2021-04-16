/*
 * RGBLeds.h
 *
 *  Created on: Jan 22, 2020
 *      Author: tyvanroy
 */

#ifndef REDLIB_RGBMOD_H_
#define REDLIB_RGBMOD_H_

#include <driverlib.h>

typedef enum
{
   RED = 2,
   GREEN = 1,
   BLUE = 0,
   ALL = 3
} rgbdevice;

typedef enum
{
   on = 1,
   off = 0,
   dim0 = 2,
   dim1 = 3
} ledSetting;

typedef enum
{
   doSend = 1,
   dontSend = 0

} sendSetting;


/*                      *
 *      LP3943 API      *
 *                      */

uint8_t RGBMod_init;

/*
 *
 * LED defaults defined in init_RGBMod():
 *
 * R = 0
 * G = 0
 * B = 0
 */
uint32_t RAW_R_DATA;
uint32_t RAW_G_DATA;
uint32_t RAW_B_DATA;

/*
 *
 * PWM defaults defined in init_RGBMod():
 *
 * CH0 = 160hZ @25% ****
 * CH1 = 160hZ @50%
 */
uint32_t RAW_R_PWM;
uint32_t RAW_G_PWM;
uint32_t RAW_B_PWM;

inline void RedFlag(void);

// transmit data w/ destination reg to LP3943 unit
void LP3943_Transmit(rgbdevice unit, uint8_t reg, uint8_t data);

// send LP3943 pwm data -> Transmit()
void LP3943_SendPWMSettings(rgbdevice unit);

// set LP3943 pwm data (channel = dim0 | dim1)
void LP3943_SetPWMSettings(rgbdevice unit, ledSetting channel, uint8_t psc, uint8_t pwm, sendSetting send);

// send LP3943 led data -> Transmit()
void LP3943_SendDATA(rgbdevice unit);    // SUPPORTS 'ALL' UNIT

// set LEDs[0:15] without modifying 0 bits
void LedSoftSet(rgbdevice unit, uint16_t data, ledSetting set, sendSetting send);

// set LEDs[0:15] without setting 0 bits low and 1 bits high
void LedHardSet(rgbdevice unit, uint16_t data, ledSetting low, ledSetting high, sendSetting send);

void SetSingleLed(rgbdevice unit, uint8_t pos, ledSetting set, sendSetting send);

// init the RGB module
void init_RGBMod();


#endif /* REDLIB_RGBMOD_H_ */
