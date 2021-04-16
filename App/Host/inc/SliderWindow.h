/*
 * SliderWindow.h
 *
 *  Created on: Apr 29, 2020
 *      Author: tyvanroy
 */

#ifndef APP_HOST_INC_SLIDERWINDOW_H_
#define APP_HOST_INC_SLIDERWINDOW_H_

#include "RedUI.h"

Window* SliderWindow;

void InitSliderWindow(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
void RenderSliderWindow(void);
int AddSlider(char* label, int* value, int minimum, int maximum);
void SetSliderFontColor(uint8_t index, uint16_t color);
void SetSliderLabel(uint8_t index, char* label);
void SetSliderBoundaries(uint8_t index, int minimum, int maximum);
void PlusButtonOnPress(void);
void MinusButtonOnPress(void);
void ChangeSliderButtonOnPress(void);


#endif /* APP_HOST_INC_SLIDERWINDOW_H_ */
