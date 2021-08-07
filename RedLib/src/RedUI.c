/*
 * RedUI.c
 *
 *  Created on: Mar 14, 2020
 *      Author: tyvanroy
 */

#include "RedLib.h"
#include "RedUI.h"
#include "RedLCD.h"
#include <LCDLib.h>
#include <G8RTOS.h>
#include <string.h>

#define TOUCH_DEBUG_ENABLED false
#define BROKEN_SCREEN_TEST false
#define WINDOW_MANAGER_PRIORITY 0xAA
#define RENDER_PER  35
#define INPUT_PRIORITY 0

#define BUTTON_OFFSET   10

static Window windows[MAX_WINDOWS];
static Window* windowHead;              // rests at bottomWindow
static uint8_t windowCount = 0;
static Button buttons[MAX_BUTTONS];
static uint8_t IDCounter = 0;
static bool FullUpdateFlag = false;

void InitRedUI(void){
    windowHead = (struct Window*) &DefaultContext;
    FocusWindow = (struct Window*) &DefaultContext;

    for(int i = 0; i < MAX_WINDOWS; ++i){
        windows[i].update = deadWindow;
    }
    for(int i = 0; i < MAX_BUTTONS; ++i){
        buttons[i].text = "I'm dead";
    }

    /** GPIO **/

    // INPUT0
    unsigned int mask = BUTTONS0 | TOUCHBIT;
    P4->DIR &= ~mask;
    P4->IFG &= ~mask;       // P4.4 IFG cleared
    P4->IE |= mask;         // Enable interrupt on P4.3
    P4->IES |= mask;        // high-to-low transition
    P4->REN |= mask;        // Pull-up resister
    P4->OUT |= mask;        // Sets res to pull-up

    // INPUT1
    P5->DIR &= ~BUTTONS1;
    P5->IFG &= ~BUTTONS1;       // P4.4 IFG cleared
    P5->IE |= BUTTONS1;         // Enable interrupt on P4.3
    P5->IES |= BUTTONS1;        // high-to-low transition
    P5->REN |= BUTTONS1;        // Pull-up resister
    P5->OUT |= BUTTONS1;        // Sets res to pull-up

    G8RTOS_InitSemaphore(&WindowInsertionMutex, 1);

    /** Add the UI threads **/
    G8RTOS_AddAPeriodicEvent(InputManager, INPUT_PRIORITY, INPUT0_IRQ);
    G8RTOS_AddAPeriodicEvent(InputManager, INPUT_PRIORITY, INPUT1_IRQ);
    G8RTOS_AddThread(WindowManager, WINDOW_MANAGER_PRIORITY, "windowmanager");
}


/** Threads **/

void WindowManager(void){

    while(1){

        SemWait(&WindowInsertionMutex);

        if(FullUpdateFlag){                 // this will be called if a window is removed, or updated while invisible.
            LCD_Clear(LCD_BLACK);

            for(int i = 0; i < windowCount; i++){
                if(windowHead->visible){

                    windowHead->update = hardUpdate;

                    Context = windowHead;               // not sure if necessary every time
                    windowHead->Render();

                    for(int i = 0; i < windowHead->buttonCount; i++){
                        windowHead->buttonHead[i].update = hardUpdate;
                        windowHead->buttonHead[i].Render(&(windowHead->buttonHead[i]));
                        windowHead->buttonHead[i].update = noUpdate;
                    }

                    windowHead->update = noUpdate;
                }

                windowHead = windowHead->next;
            }

            FullUpdateFlag = false;
        }else{
            for(int i = 0; i < windowCount; i++){           // render the windows.
                windowHead = windowHead->next;

                if(windowHead->update > noUpdate){
                    if(!windowHead->remove){                // update window

                        if(windowHead->visible){
                            Context = windowHead;           // not sure if necessary every time
                            windowHead->Render();

                            // if hardUpdate - render buttons
                            // else render buttons if updateMode > noupdate
                            for(int i = 0; i < windowHead->buttonCount; i++){

                                if(windowHead->update == hardUpdate || windowHead->buttonHead[i].update > noUpdate){
                                    if(windowHead->buttonHead[i].update == noUpdate || windowHead->update == hardUpdate){
                                        windowHead->buttonHead->update = hardUpdate;
                                    }
                                    windowHead->buttonHead[i].Render(&(windowHead->buttonHead[i]));
                                    windowHead->buttonHead[i].update = noUpdate;
                                }
                            }

                            windowHead->update = noUpdate;
                        }
                    }else{                                  // remove window
                        Window* prev = windowHead->prev;
                        prev->next = windowHead->next;
                        windowHead->next->prev = prev;

                        windowHead->update = deadWindow;

                        windowCount--;

                        windowHead = windowHead->next;
                        FullUpdateFlag = true;
                        continue;
                    }


                }
            }
        }
        SemSignal(&WindowInsertionMutex);
        sleep(RENDER_PER);
    }
}

void InputManager(void){
    NVIC_DisableIRQ(INPUT0_IRQ);
    NVIC_DisableIRQ(INPUT1_IRQ);

    INPUT0_IFG &= ~(BUTTONS0 | TOUCHBIT);
    INPUT1_IFG &= ~BUTTONS1;

    if(BROKEN_SCREEN_TEST){
        Point tp = {0, 0};
        TP_ReadXY(&tp);
        ScaleTP(&tp);
        int tx = tp.x;
        int ty = tp.y;
        Context = &DefaultContext;
        if(tx > 4032 || ty <= 96){        // this is an empty reading - no touch on trigger
            DrawRect(0, 0, Context->width, Context->height, 10, LCD_RED);
            Write(20, 20, "NO TOUCH ON TRIGGER", 1, LCD_RED);
        }
    }

    if(InputPressed(TOUCH)){
        MaskInput(TOUCH);

        NVIC_EnableIRQ(INPUT0_IRQ);
        NVIC_EnableIRQ(INPUT1_IRQ);

        G8RTOS_AddThread(TouchManager, 0, "touchmanager");
        return;
    }else{
        FocusWindow->inputField = GetInputField();
        FocusWindow->inputFlag = true;
    }

    NVIC_EnableIRQ(INPUT0_IRQ);
    NVIC_EnableIRQ(INPUT1_IRQ);
}

void TouchManager(void){

    // Use the whole window to get touch coordinates.
    FocusWindow = &DefaultContext;

    // get the decoded coordinates
    Point tp = {0, 0};
    TP_ReadXY(&tp);
    ScaleTP(&tp);
    int tx = tp.x;
    int ty = tp.y;

    if(TOUCH_DEBUG_ENABLED){
        char xstr[10];
        char ystr[10];
        snprintf((xstr), 10, "x: %d", tx);
        snprintf((ystr), 10, "y: %d", ty);

        ClearTextLines(tx, ty, 2, 10, LCD_BLACK);
        Write(tx, ty, xstr, 1, LCD_WHITE);
        Write(tx, ty + 16, ystr, 1, LCD_WHITE);
        DrawPoint(tx, ty, LCD_WHITE);
    }

    // find the touched window
    Window* target = windowHead;
    int inBoundsHighest = -1;
    for(int i = 0; i < windowCount; i++){
        if((tx >= target->x && tx < (target->x + target->width)) &&     // compare the touch point with the window's boundaries.
           (ty >= target->y && ty < (target->y + target->height)) &&
           (target->layer > inBoundsHighest) &&
           (target->visible)){                           // check the window's layer

            inBoundsHighest = target->layer;
            break;
        }
        target = target->next;
    }

    if(inBoundsHighest == -1){
        target = (Window*) &DefaultContext;
    }

    // set as the FocusWindow
    GiveFocus(target);

    // find touched button
    Button* button = FocusWindow->buttonHead;
    inBoundsHighest = -1;
    for(int i = 0; i < FocusWindow->buttonCount; i++){
        if((tx >= ((button->x + FocusWindow->x) - BUTTON_OFFSET) && tx < ((button->x + FocusWindow->x) + button->width) - BUTTON_OFFSET) &&     // compare the touch point with the window's boundaries.
           (ty >= ((button->y + FocusWindow->y) - BUTTON_OFFSET) && ty < ((button->y + FocusWindow->y) + button->height) - BUTTON_OFFSET)){
            inBoundsHighest = i;
            break;
        }
        button = button->next;
    }

    if(inBoundsHighest != -1){
        // Flag the button, call SoftUpdate on its parent.
        button->pressed = true;
        Flag(button);

        sleep(LUCKY);           // debounce

        G8RTOS_AddThread(button->OnPress, 0, "button onpress");
    }else{
        // if it wasn't a button, add/call Window.OnTouch thread
        FocusWindow->inputField |= TOUCH;
        FocusWindow->inputFlag = true;
        FocusWindow->touchPoint.x = tx;
        FocusWindow->touchPoint.y = ty;

        sleep(LUCKY);           // debounce

        G8RTOS_AddThread(target->OnTouch, 0, "window ontouch");
    }

    G8RTOS_KillSelf();
    while(1);
}

void DefaultOnTouch(void){
    Point tp = {0, 0};

    while(InputPressed(TOUCH)){
        TP_ReadXY(&tp);
        ScaleTP(&tp);

        FocusWindow->touchPoint.x = tp.x;
        FocusWindow->touchPoint.y = tp.y;

        sleep(FocusWindow->trackingDelay);
    }

    FinishOnTouch();
}

void FinishOnTouch(void){
    FocusWindow->touchPoint.x = -1;
    FocusWindow->touchPoint.y = -1;
    FocusWindow->inputField &= ~(TOUCH);
    FocusWindow->inputFlag = false;
    UnmaskInput(TOUCH);
    G8RTOS_KillSelf();
    while(1);
}

void FinishOnPress(void){
    UnmaskInput(TOUCH);
    G8RTOS_KillSelf();
    while(1);
}

void DefaultButtonRender(Button* button){

    // hard update
    if(button->update > softUpdate){

        FillRect(button->x, button->y, button->width, button->height, button->bgColor);

        int tx = (button->x + (button->width >> 1)) - ((button->textLength * (button->fontSize << 3)) >> 1);
        int ty = (button->y + (button->height >> 1)) - (button->fontSize << 3);
        Write(tx, ty, button->text, button->fontSize, button->fontColor);

        if(button->pressed){
            DrawRect(button->x, button->y, button->width, button->height, button->outlineThickness << 1, button->outlineColor);
        }else{
            DrawRect(button->x, button->y, button->width, button->height, button->outlineThickness, button->outlineColor);
        }

        return;
    }


    // else, the button has been pressed or released

    if(button->pressed){    // it was pressed
        DrawRect(button->x, button->y, button->width, button->height, button->outlineThickness << 1, button->outlineColor);
    }else{                  // it was released
        DrawRect(button->x, button->y, button->width, button->height, button->outlineThickness << 1, button->bgColor);
        DrawRect(button->x, button->y, button->width, button->height, button->outlineThickness, button->outlineColor);
    }

}


/** API **/

ID_t AddWindow(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t layer, void (*RenderFunction)(void)){
    SemWait(&WindowInsertionMutex);

    // check availability
    if(windowCount >= MAX_WINDOWS){
        return (ID_t) -1;
    }

    int pos = 0;

    // add it
    if(windowCount == 0){
        windows[pos].next = &windows[pos];
        windows[pos].prev = &windows[pos];

        windowHead = &windows[pos];
    }else{
        // find first dead window
        while(windows[pos].update != deadWindow){
            pos++;
        }

        // another out of bounds check
        if(pos >= MAX_WINDOWS){
            return (ID_t) -1;
        }

        // find the last window with layer beneath this one
        Window* insertAfterThis = windowHead;

        if(insertAfterThis->layer > layer){
            // insert before insertAfterThis
            windows[pos].next = insertAfterThis;
            windows[pos].prev = insertAfterThis->prev;
            insertAfterThis->prev->next = &windows[pos];
            insertAfterThis->prev = &windows[pos];
        }else{

            // insert after insertAfterThis
            for(int i = 0; i < windowCount; i++){
                if(insertAfterThis->layer <= layer && insertAfterThis->next->layer > layer){
                    break;
                }

                insertAfterThis = i == 3 ? insertAfterThis : insertAfterThis->next;
            }

            windows[pos].prev = insertAfterThis;
            windows[pos].next = insertAfterThis->next;
            insertAfterThis->next->prev = &windows[pos];
            insertAfterThis->next = &windows[pos];
        }
    }

    // initialize
    windows[pos].x = x;
    windows[pos].y = y;
    windows[pos].width = width;
    windows[pos].height = height;
    windows[pos].layer = layer;
    windows[pos].Render = RenderFunction;
    windows[pos].OnTouch = DefaultOnTouch;
    windows[pos].trackingDelay = DEFAULT_TRACKING_DELAY;
    windows[pos].update = noUpdate;
    windows[pos].visible = false;
    windows[pos].remove = false;
    windows[pos].inputFlag = false;
    windows[pos].inputField = 0;
    windows[pos].touchPoint.x = -1;
    windows[pos].touchPoint.y = -1;
    windows[pos].buttonCount = 0;
    windows[pos].ID = ((IDCounter++) << 16) | windowCount;

    windowCount++;

    SemSignal(&WindowInsertionMutex);
    return windows[pos].ID;
}

Window* GetWindow(ID_t ID){
    Window* target = windowHead;
    bool found = false;

    for(int i = 0; i < windowCount; i++){
        if(target->ID == ID){
            found = true;
            break;
        }
        target = target->next;
    }

    if(!found){
        return (Window*) &DefaultContext;
    }

    return target;
}

inline void SoftUpdate(Window* window){
    window->update = window->update == hardUpdate ? hardUpdate : softUpdate;
}

inline void HardUpdate(Window* window){
    window->update = hardUpdate;
}

inline void GiveFocus(Window* window){
    FocusWindow->inputField = 0;
    FocusWindow->inputFlag = false;
    UnmaskField(0xf);
    FocusWindow = window;
}

void SetOnTouchHandler(Window* window, void(*OnTouch)(void)){
    window->OnTouch = OnTouch;
}

void SetWindowRenderFunction(Window* window, void(*RenderFunction)()){
    SemWait(&WindowInsertionMutex);
    window->Render = RenderFunction;
    SemSignal(&WindowInsertionMutex);
}

void SetTrackingDelay(Window* window, uint16_t trackingDelay){
    window->trackingDelay = trackingDelay;
}

void SetLayer(Window* window, uint8_t layer){
    if(windowCount > 0){
        SemWait(&WindowInsertionMutex);

        // remove from list
        Window* prev = window->prev;
        prev->next = window->next;
        window->next->prev = prev;

        // find the last window with layer beneath this one
        Window* insertAfterThis = windowHead;

        if(insertAfterThis->layer > layer){
            // insert before insertAfterThis
            window->next = insertAfterThis;
            window->prev = insertAfterThis->prev;
            insertAfterThis->prev->next = window;
            insertAfterThis->prev = window;
        }else{

            // insert after insertAfterThis
            for(int i = 0; i < windowCount; i++){
                if(insertAfterThis->layer <= layer && insertAfterThis->next->layer > layer){
                    break;
                }
                insertAfterThis = i == 3 ? insertAfterThis : insertAfterThis->next;
            }
            window->prev = insertAfterThis;
            window->next = insertAfterThis->next;
            insertAfterThis->next->prev = window;
            insertAfterThis->next = window;
        }
        SemSignal(&WindowInsertionMutex);
    }

    window->layer = layer;
    window->update = hardUpdate;
}

void SetVisible(Window* window, bool visible){
    window->visible = visible;
    FullUpdateFlag = true;
}

void RemoveWindow(Window* window){
    window->remove = true;
    window->update = hardUpdate;
}

ID_t AddButton(uint16_t x, uint16_t y, uint16_t width, uint16_t height, void (*OnPress)(void), Window* parent){
    SemWait(&WindowInsertionMutex);

    // find first available window
    int pos = 0;
    while(strcmp(buttons[pos].text, "I'm dead")){
        pos++;
    }

    if(pos >= MAX_BUTTONS){
        return (ID_t) -1;
    }

    buttons[pos].parent = parent;

    // add it
    if(parent->buttonCount == 0){
        buttons[pos].next = &buttons[pos];
        buttons[pos].prev = &buttons[pos];

        parent->buttonHead = &buttons[pos];
    }else{

        buttons[pos].prev = parent->buttonHead;
        buttons[pos].next = parent->buttonHead->next;
        parent->buttonHead->next->prev = &buttons[pos];
        parent->buttonHead->next = &buttons[pos];

    }

    // initialize
    buttons[pos].x = x;
    buttons[pos].y = y;
    buttons[pos].width = width;
    buttons[pos].height = height;
    buttons[pos].bgColor = DEFAULT_BUTTON_BG;
    buttons[pos].outlineColor = DEFAULT_BUTTON_OUTLINE_COLOR;
    buttons[pos].fontColor = DEFAULT_BUTTON_FONT;
    buttons[pos].fontSize = 1;
    buttons[pos].outlineThickness = DEFAULT_BUTTON_OUTLINE_THICKNESS;
    buttons[pos].text = "";
    buttons[pos].textLength = 0;
    buttons[pos].Render = DefaultButtonRender;
    buttons[pos].OnPress = OnPress;
    buttons[pos].visible = true;
    buttons[pos].remove = false;
    buttons[pos].update = hardUpdate;
    buttons[pos].pressed = false;
    buttons[pos].ID = ((IDCounter++) << 16) | windowCount;

    parent->buttonCount++;

    SemSignal(&WindowInsertionMutex);
    return buttons[pos].ID;
}

Button* GetButton(ID_t ID){
    for(int i = 0; i < MAX_BUTTONS; i++){
        if(buttons[i].ID == ID){
            return &buttons[i];
        }
    }

    return NULL;
}

void Flag(Button* button){
    button->update = softUpdate;
    SoftUpdate(button->parent);
}

void SetOnPressHandler(Button* button, void(*OnPress)(void)){
    button->OnPress = OnPress;
}

void SetButtonRenderFunction(Button* button, void(*RenderFunction)(Button* button)){
    button->Render = RenderFunction;
}

void SetButtonText(Button* button, char* text){
    button->textLength = strlen(text);
    button->text = text;
    HardUpdate(button->parent);
}

void SetButtonBackgroundColor(Button* button, uint16_t color){
    button->bgColor = color;
    button->update = hardUpdate;
    SoftUpdate(button->parent);
}

void SetButtonOutlineColor(Button* button, uint16_t color){
    button->outlineColor = color;
    button->update = hardUpdate;
    SoftUpdate(button->parent);
}

void SetButtonFontColor(Button* button, uint16_t color){
    button->fontColor = color;
    button->update = hardUpdate;
    SoftUpdate(button->parent);
}

void SetButtonFontSize(Button* button, uint8_t size){
    button->fontSize = size;
    button->update = hardUpdate;
    SoftUpdate(button->parent);
}

void SetButtonOutlineThickness(Button* button, uint8_t thickness){
    button->outlineThickness = thickness;
    button->update = hardUpdate;
    SoftUpdate(button->parent);
}

void RemoveButton(Button* button){
    button->remove = true;
    button->update = hardUpdate;
    HardUpdate(button->parent);
}

void SetButtonVisible(Button* button, bool visible){
    button->visible = visible;
    button->update = hardUpdate;
    HardUpdate(button->parent);
}

inline uint8_t GetInputField(void){
    uint8_t field =
            (!(INPUT0 & B0_PIN) ? B0 : 0) |
            (!(INPUT0 & B1_PIN) ? B1 : 0) |
            (!(INPUT1 & B2_PIN) ? B2 : 0) |
            (!(INPUT1 & B3_PIN) ? B3 : 0) |
            (!(INPUT0 & B4_PIN) ? B4 : 0) |
            (!(INPUT0 & TOUCHBIT) ? TOUCH : 0);
    return field;
}

inline bool InputPressed(Input b){
    return (GetInputField() & b);
}

void MaskInput(Input b){
    switch(b){
        case TOUCH:
            INPUT0_MASK &= ~TOUCHBIT;
            break;
        case B0:
            INPUT0_MASK &= ~BIT4;
            break;
        case B1:
            INPUT0_MASK &= ~BIT5;
            break;
        case B2:
            INPUT1_MASK &= ~BIT4;
            break;
        case B3:
            INPUT1_MASK &= ~BIT5;
            break;
        case B4:
            INPUT0_MASK &= ~BIT3;
            break;
    }
}

void UnmaskField(uint8_t field){
    if(field & TOUCH){
        INPUT0_IFG &= ~TOUCHBIT;
        INPUT0_MASK |= TOUCHBIT;
    }
    if(field & B0){
        INPUT0_IFG &= ~BIT4;
        INPUT0_MASK |= BIT4;
    }
    if(field & B1){
        INPUT0_IFG &= ~BIT5;
        INPUT0_MASK |= BIT5;
    }
    if(field & B2){
        INPUT1_IFG &= ~BIT4;
        INPUT1_MASK |= BIT4;
    }
    if(field & B3){
        INPUT1_IFG &= ~BIT5;
        INPUT1_MASK |= BIT5;
    }
    if(field & B4){
        INPUT0_IFG &= ~BIT3;
        INPUT0_MASK |= BIT3;
    }
}

void UnmaskInput(Input b){
    UnmaskField(b);
}

uint8_t DefaultInputField(void){
    return DefaultContext.inputField;
}

bool DefaultFlag(void){
    return DefaultContext.inputFlag;
}

