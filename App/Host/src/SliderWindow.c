/*
 * SliderWindow.c
 *
 *  Created on: Apr 29, 2020
 *      Author: tyvanroy
 */

#include "SliderWindow.h"
#include "HostMenu.h"
#include "World.h"
#include <string.h>

#define MAX_VALUES              15

#define PADDING                 10
#define LABEL_HEIGHT            20
#define BUTTON_HEIGHT           30
#define VALUE_HEIGHT            30

typedef struct SliderData{
    char* label;
    uint8_t labelLength;
    int* value;
    int minimum;
    int maximum;
    uint16_t fontColor;
} SliderData;

static Button* PlusButton;
static Button* MinusButton;
static Button* ChangeSliderButton;

static SliderData Sliders[MAX_VALUES];
static uint8_t SliderCount;
static uint8_t ActiveSlider;
static char ValueBuffer[4];

void InitSliderWindow(uint16_t x, uint16_t y, uint16_t width, uint16_t height){

    SliderWindow = GetWindow(AddWindow(x, y, width, height, TOP_LAYER, RenderSliderWindow));

    MinusButton = GetButton(AddButton(0, LABEL_HEIGHT, BUTTON_HEIGHT, BUTTON_HEIGHT, MinusButtonOnPress, SliderWindow));
    SetButtonText(MinusButton, "-");
    SetButtonFontSize(MinusButton, 2);

    PlusButton = GetButton(AddButton(SliderWindow->width - (BUTTON_HEIGHT << 1), LABEL_HEIGHT, BUTTON_HEIGHT, BUTTON_HEIGHT, PlusButtonOnPress, SliderWindow));
    SetButtonText(PlusButton, "+");
    SetButtonFontSize(PlusButton, 2);

    ChangeSliderButton = GetButton(AddButton(SliderWindow->width - (BUTTON_HEIGHT), LABEL_HEIGHT, BUTTON_HEIGHT, BUTTON_HEIGHT, ChangeSliderButtonOnPress, SliderWindow));
    SetButtonText(ChangeSliderButton, ">>");

    SliderCount = 0;

    SetVisible(SliderWindow, true);
}

int AddSlider(char* label, int* value, int minimum, int maximum){
    if(SliderCount >= MAX_VALUES){
        return -1;
    }

    Sliders[SliderCount].value = value;
    Sliders[SliderCount].labelLength = strlen(label);
    Sliders[SliderCount].label = label;
    Sliders[SliderCount].minimum = minimum;
    Sliders[SliderCount].maximum = maximum;
    Sliders[SliderCount].fontColor = LCD_WHITE;

    SliderCount++;

    return (SliderCount - 1);
}

void SetSliderFontColor(uint8_t index, uint16_t color){
    Sliders[index].fontColor = color;
    SoftUpdate(SliderWindow);
}

void SetSliderLabel(uint8_t index, char* label){
    Sliders[index].labelLength = strlen(label);
    Sliders[index].label = label;
    HardUpdate(SliderWindow);
}

void SetSliderBoundaries(uint8_t index, int minimum, int maximum){
    Sliders[index].minimum = minimum;
    Sliders[index].maximum = maximum;

    // Check to see if value is still in bounds
}

void PlusButtonOnPress(void){
    while(InputPressed(TOUCH)){
        sleep(1);
    }

    if((*Sliders[ActiveSlider].value) <= (Sliders[ActiveSlider].maximum - 1)){
        (*Sliders[ActiveSlider].value)++;
    }else{
        (*Sliders[ActiveSlider].value) = Sliders[ActiveSlider].minimum;
    }
    PlusButton->pressed = false;
    Flag(PlusButton);                // flag for rendering
    SoftUpdate(SliderWindow);
    sleep(40);

    FinishOnPress();
}

void MinusButtonOnPress(void){
    while(InputPressed(TOUCH)){
        sleep(1);
    }


    if((*Sliders[ActiveSlider].value) >= (Sliders[ActiveSlider].minimum + 1)){
        (*Sliders[ActiveSlider].value)--;
    }else{
        (*Sliders[ActiveSlider].value) = Sliders[ActiveSlider].maximum;
    }
    MinusButton->pressed = false;
    Flag(MinusButton);                // flag for rendering

    sleep(40);
    SoftUpdate(SliderWindow);

    FinishOnPress();
}

void ChangeSliderButtonOnPress(void){
    while(InputPressed(TOUCH)){
        sleep(1);
    }

    if(ActiveSlider >= SliderCount - 1){
        ActiveSlider = 0;
    }else{
        ActiveSlider++;
    }
    ChangeSliderButton->pressed = false;
    Flag(ChangeSliderButton);                // flag for rendering

    HardUpdate(SliderWindow);

    sleep(40);

    FinishOnPress();
}

void RenderSliderWindow(void){
    if(Context->update == hardUpdate){
        int tx = (Context->width >> 1) - (Sliders[ActiveSlider].labelLength << 1);
        FillRect(0, 0, Context->width, LABEL_HEIGHT, LCD_BLACK);
        Write(tx, 0, Sliders[ActiveSlider].label, 1, LCD_WHITE);
    }

    sprintf(ValueBuffer, "%d", (*Sliders[ActiveSlider].value));
    int len = strlen(ValueBuffer);
    FillRect(BUTTON_HEIGHT, LABEL_HEIGHT, Context->width - (BUTTON_HEIGHT * 3), BUTTON_HEIGHT, LCD_BLACK);
    Write(((Context->width - BUTTON_HEIGHT) >> 1) - ((len << 3) >> 1), LABEL_HEIGHT, ValueBuffer, 1, Sliders[ActiveSlider].fontColor);
}


