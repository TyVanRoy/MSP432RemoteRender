/*
 * RedUI.h
 *
 * This module adds 1 threads to G8RTOS:
 *  WindowManager (background) with priority WINDOW_MANAGER_PRIORITY
 *
 * It also adds InputManager twice, under INPUT0_IRQ & INPUT1_IRQ
 * with priority INPUT_PRIORITY
 *
 *
 *  NOTE: If you use this module, do not attempt to write
 *      to the LCD outside of functions passed to AddWindow( as renderThread)  !
 *
 *  Created on: Mar 14, 2020
 *      Author: tyvanroy
 */

#ifndef REDLIB_INC_REDUI_H_
#define REDLIB_INC_REDUI_H_

#include "RedTypes.h"
#include <G8RTOS_Semaphores.h>

#define MAX_WINDOWS 15
#define MAX_BUTTONS 30

#define BOTTOM_LAYER 0
#define TOP_LAYER 255

#define DEFAULT_TRACKING_DELAY 30

#define DEFAULT_BUTTON_BG LCD_WHITE
#define DEFAULT_BUTTON_OUTLINE_COLOR LCD_BLACK
#define DEFAULT_BUTTON_OUTLINE_THICKNESS 1
#define DEFAULT_BUTTON_FONT LCD_BLACK

semaphore_t WindowInsertionMutex;   // ensures layers are never altered while the WindowManager is operating.

/**
 *  This is used for RedLCD's Context if there are no alive Windows.
 */
Window DefaultContext;

/**
 * This window is used for Input routing.
 *  - When the top-layer window at the touch location is touched, it gets focus.
 *  - This same window is also the window who's buttons/OnTouch function are called.
 *  - When the tactile buttons are pressed, FocusWindow's flags are set.
 *
 * Also, the ScaleTP functions will translate their coordinates by this Window's position.
 */
Window* FocusWindow;

/** MUST CALL G8RTOS_Init BEFORE CALLING THIS **/
void InitRedUI(void);

/** Threads **/

/**
 *  This thread synchronously manages LCD rendering so we don't have to think about it.
 *  If you use this manager, be careful about writing to the LCD in a non-render thread.
 *
 *  Windows will never be touched if update == noUpdate, even if removed or set invisible!
 *
 **/
void WindowManager(void);

/**
 *  This is an interrupt handler for Port4.
 *
 *  It masks the interrupt bit called, then sets the appropriate flag
 *  in the focused context.
 *
 *  The default context has focus by default.
 *
 *  Note: The window with focus is responsible for unmasking the interrupts.
 *    *   Also, tactile buttons are not debounced automatically!
 *
 **/
void InputManager(void);

/**
 *  This thread is spawned when the screen is touched and available.
 *  It debounces the touch, then calls the appropriate handler.
 *
 *  Calls KillSelf() when done.
 *
 *  Note: The TouchHandler called is responsible for unmasking the TOUCH Button.
 **/
void TouchManager(void);

/** Call this function at the end of your custom OnTouch functions! */
void FinishOnTouch(void);
/** Call this function at the end of your custom OnPress functions! */
void FinishOnPress(void);

/** This thread is what handles a window's touch input by default
 *  if it is not a button that is being pressed.
 */
void DefaultOnTouch(void);

/** The default button render function **/
void DefaultButtonRender(Button* button);


/** API **/

/** Windows **/
/**
 * Returns the new window's ID (Dont lose it!)
 *
 * Window is not visible by default!
 **/
ID_t AddWindow(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t layer, void (*RenderFunction)(void));

/**
 * Get a pointer to a Window
 *
 * Returns DefaultContext if ID is not found
 *
 **/
Window* GetWindow(ID_t ID);
inline void SoftUpdate(Window* window);
inline void HardUpdate(Window* window);
inline void GiveFocus(Window* window);

/**
 * Sets the Window's OnTouch function.
 *
 * NOTE: Make sure to call FinishOnTouch at the end of your OnTouch function!
 **/
void SetOnTouchHandler(Window* window, void(*OnTouch)(void));
void SetWindowRenderFunction(Window* window, void(*RenderFunction)());
void SetTrackingDelay(Window* window, uint16_t trackingDelay);

/** Sets the property, calls a hardUpdate **/
void SetLayer(Window* window, uint8_t layer);   // resorts the window list before calling hardUpdate.
void SetVisible(Window* window, bool visible);
void RemoveWindow(Window* window);


/** Buttons **/
ID_t AddButton(uint16_t x, uint16_t y, uint16_t width, uint16_t height, void (*OnPress)(void), Window* parent);
Button* GetButton(ID_t ID);

/**
 *
 *  This function flags the button for rendering by setting SoftUpdate on the button and its parent.
 *
 *  Only use this function to alter the updateMode of the button.
 *  Don't set button->updateMode manually.
 *
 **/
void Flag(Button* button);

void SetOnPressHandler(Button* button, void(*OnPress)(void));
void SetButtonRenderFunction(Button* button, void(*RenderFunction)(Button* button));

/** Sets the property, flags the button. **/
void SetButtonText(Button* button, char* text);
void SetButtonBackgroundColor(Button* button, uint16_t color);
void SetButtonOutlineColor(Button* button, uint16_t color);
void SetButtonFontColor(Button* button, uint16_t color);
void SetButtonFontSize(Button* button, uint8_t size);
void SetButtonOutlineThickness(Button* button, uint8_t thickness);

/** Sets the property, flags the button for rendering, calls a hardUpdate on parent window. **/
void SetButtonVisible(Button* button, bool visible);
void RemoveButton(Button* button);

/** GENERAL INPUT **/
inline uint8_t GetInputField(void);
inline bool InputPressed(Input b);

/** Masks/Unmasks the associated interrupt vector **/
void MaskInput(Input b);
void UnmaskField(uint8_t field);
void UnmaskInput(Input b);

/** Returns the button flags from Default Context **/
uint8_t DefaultInputField(void);
bool DefaultFlag(void);

#endif /* REDLIB_INC_REDUI_H_ */
