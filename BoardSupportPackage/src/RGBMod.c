/*
 * RGBLeds.c
 *
 *  Created on: Jan 22, 2020
 *      Author: tyvanroy
 */

#include "RGBMod.h"

inline void RedFlag(void){
    LedHardSet(RED, 0xffff, off, on, doSend);
}

void LP3943_Transmit(rgbdevice unit, uint8_t reg, uint8_t data){

    if(unit == ALL)
        return;

    UCB2IFG = 0;            // clear int-flags

    UCB2I2CSA = (unit | 0b11100000);     // set slave address

    UCB2CTLW0 |= UCTR;      // transmitter mode

    UCB2CTLW0 |= UCTXSTT;   // start condition

    // transmit data

    while(!(UCB2IFG & UCTXIFG0));
    UCB2TXBUF = reg; // reg

    while(!(UCB2IFG & UCTXIFG0));
    UCB2TXBUF = data; // data

    while(!(UCB2IFG & UCTXIFG0));
    UCB2CTLW0 |= UCTXSTP;       // stop condition
}

void LP3943_SendPWMSettings(rgbdevice unit)
{
    if(unit == ALL)
            return;

    int i;
    uint32_t data;
    uint32_t bigMask = 0xff;

    switch(unit){
    case RED: data = RAW_R_PWM;
        break;
    case GREEN: data = RAW_G_PWM;
        break;
    case BLUE: data = RAW_B_PWM;
        break;
    default: data = 0;
    }

    for(i = 0; i < 4; i++){
        LP3943_Transmit(unit, 0x02 + i, (uint8_t) (0xff & ((bigMask & data) >> (8 * i))));

        bigMask = bigMask << 8;
    }
}

void LP3943_SetPWMSettings(rgbdevice unit, ledSetting channel, uint8_t psc, uint8_t pwm, sendSetting send){
    if((channel != dim0 && channel != dim1))
        return;

    if(unit == ALL)
            return;

    uint32_t dataMask = 0x0000ffff;
    uint32_t vData;
    uint32_t* unitData;

    switch(unit){
    case RED: unitData = &RAW_R_PWM;
       break;
    case GREEN: unitData = &RAW_G_PWM;
       break;
    case BLUE: unitData = &RAW_B_PWM;
       break;
    default: unitData = 0;
    }

    channel -= 2;
    dataMask = (dataMask << (channel * 16));
    vData = ((((uint32_t) pwm) << 8 | psc) << (channel * 16));

    *unitData &= (~dataMask);
    *unitData |= vData;

    if(send)
        LP3943_SendPWMSettings(unit);
}

void LP3943_SendData(rgbdevice unit){
    int i;
    uint32_t data;

    if(unit == ALL){

        for(i = 0; i < 4; i++){
            LP3943_Transmit(RED, 0x06 + i, 0xff & (RAW_R_DATA >> (8 * i)));
        }
        for(i = 0; i < 4; i++){
            LP3943_Transmit(GREEN, 0x06 + i, 0xff & (RAW_G_DATA >> (8 * i)));
        }
        for(i = 0; i < 4; i++){
            LP3943_Transmit(BLUE, 0x06 + i, 0xff & (RAW_B_DATA >> (8 * i)));
        }

        return;
    }

    switch(unit){
    case RED: data = RAW_R_DATA;
        break;
    case GREEN: data = RAW_G_DATA;
        break;
    case BLUE: data = RAW_B_DATA;
        break;
    default: data = 0;
    }

    uint32_t mask = 0x000000ff;

    // add flag system
    for(i = 0; i < 4; i++){
        LP3943_Transmit(unit, 0x06 + i, (uint8_t) (0xff & ((mask & data) >> (8 * i))));
        mask = mask << 8;
    }

}

void LedSoftSet(rgbdevice unit, uint16_t data, ledSetting set, sendSetting send)
{
   int i;
   uint32_t dataMask = 0;
   uint32_t aataMask = 0;
   uint16_t posMask = 0x8000;

   uint32_t* unitData;

   switch(unit){
   case RED: unitData = &RAW_R_DATA;
       break;
   case GREEN: unitData = &RAW_G_DATA;
       break;
   case BLUE: unitData = &RAW_B_DATA;
       break;
   default: unitData = 0;
   }

   for(i = 0; i < 16; i++){
       if(posMask & data){
           dataMask = set << (i * 2);
           aataMask = 0x00000003 << (i * 2);

           *unitData &= (~aataMask);
           *unitData |= (dataMask);
       }

       posMask = posMask >> 1;
   }

    if(send)
        LP3943_SendData(unit);
}

void LedHardSet(rgbdevice unit, uint16_t data, ledSetting low, ledSetting high, sendSetting send)
{
    if(unit == ALL)
            return;

   int i;
   uint32_t vData = 0;
   uint16_t posMask = 0x8000;

   uint32_t* unitData;

   switch(unit){
   case RED: unitData = &RAW_R_DATA;
       break;
   case GREEN: unitData = &RAW_G_DATA;
       break;
   case BLUE: unitData = &RAW_B_DATA;
       break;
   default: unitData = 0;
   }

   for(i = 0; i < 16; i++){
       vData |= (((posMask & data) ? (high) : (low)) << (2 * i));

       posMask = posMask >> 1;
   }

   *unitData = vData;

    if(send)
        LP3943_SendData(unit);
}

void SetSingleLed(rgbdevice unit, uint8_t pos, ledSetting set, sendSetting send){
    if(unit == ALL)
        return;

    uint32_t* unitData;

   switch(unit){
   case RED: unitData = &RAW_R_DATA;
       break;
   case GREEN: unitData = &RAW_G_DATA;
       break;
   case BLUE: unitData = &RAW_B_DATA;
       break;
   default: unitData = 0;
   }

   pos %= 16;
   uint32_t dataMask = set << (pos * 2);
   uint32_t aataMask = 0x00000003 << (pos * 2);

   *unitData &= (~aataMask);
   *unitData |= (dataMask);

   if(send)
       LP3943_SendData(unit);
}

void init_RGBMod()
{
    UCB2CTLW0 = UCSWRST;                // software reset enable

    // Initialize I2C master
    // Set as master, I2C mode, clock sync, SMCLK source, transmitter
    UCB2CTLW0 |= (UCMST | (0x000011 << 9) | UCSYNC | (0x0011 << 6)); // finish

    // Set the Fclk to 400khz
    // Assumes SMCLK is selected as source and Fsmclk is 12Mhz
    UCB2BRW = 30;

    // Sets pins as I2C mode
    // Set P3.6 AS UCB2_SDA AND P3.7 AS UCB2_SLC
    P3SEL0 |= 0b11000000; //
    P3SEL1 &= 0x00; //

    UCB2CTLW0 &= ~UCSWRST;

    RAW_R_DATA = 0x00;
    RAW_G_DATA = 0x00;
    RAW_B_DATA = 0x00;

    RAW_R_PWM = 0x00;
    RAW_G_PWM = 0x00;
    RAW_B_PWM = 0x00;

    // set pwm
    /*
    LP3943_SetPWMSettings(RED, dim0, 0x9f, 0x40, dontSend);
    LP3943_SetPWMSettings(RED, dim1, 0x9f, 0x80, doSend);
    LP3943_SetPWMSettings(GREEN, dim0, 0x9f, 0x40, dontSend);
    LP3943_SetPWMSettings(GREEN, dim1, 0x9f, 0x80, doSend);
    LP3943_SetPWMSettings(BLUE, dim0, 0x9f, 0x40, dontSend);
    LP3943_SetPWMSettings(BLUE, dim1, 0x9f, 0x80, doSend);
    */

    RGBMod_init = 1;
}
