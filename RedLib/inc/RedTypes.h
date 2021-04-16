/*
 * RedTypes.h
 *
 *  Created on: Mar 16, 2020
 *      Author: tyvanroy
 */

#ifndef REDLIB_INC_REDTYPES_H_
#define REDLIB_INC_REDTYPES_H_

#include <stdbool.h>
#include <stdint.h>
#include <LCDLib.h>

#define PI          3.14159265359
#define HALF_PI     1.570796327
#define LUCKY       333

#define MAX_FRAME_LINES 50
#define MAX_POLY_POINTS 12
#define MAX_POLY_LINES  16

/** 2D **/

typedef struct Pair{
    uint8_t a;
    uint8_t b;
} Pair;

typedef struct Line {
    Point a;
    Point b;
    uint16_t color;
} Line;

typedef struct Frame_t{
    uint8_t lineCount;
    Line lines[MAX_FRAME_LINES];
} Frame_t;


/** 3D **/

typedef struct Point3{
    float x;
    float y;
    float z;
} Point3;

/* 4x4 matrix */
typedef struct Matrix{
    float data[16];
} Matrix;

typedef struct Camera {
    Point3 position;
    Point3 rotation;
} Camera;

typedef enum Axis{
    xaxis,
    yaxis,
    zaxis
} Axis;

typedef struct Poly{
    uint8_t pointCount;
    Point3 points[MAX_POLY_POINTS];
    uint8_t lineCount;
    Pair lines[MAX_POLY_LINES];
} Poly;


/** UI **/

#define INPUT0 P4->IN
#define INPUT1 P5->IN
#define INPUT0_IFG P4->IFG
#define INPUT1_IFG P5->IFG
#define INPUT0_MASK P4->IE
#define INPUT1_MASK P5->IE
#define INPUT0_IRQ PORT4_IRQn
#define INPUT1_IRQ PORT5_IRQn

#define BUTTONS0 (BIT4 | BIT5 | BIT3)
#define BUTTONS1 (BIT4 | BIT5)
#define TOUCHBIT BIT0
#define B0_PIN BIT4
#define B1_PIN BIT5
#define B2_PIN BIT4
#define B3_PIN BIT5
#define B4_PIN BIT3

typedef uint32_t ID_t;

typedef enum UpdateMode{
    noUpdate = 0,
    softUpdate = 1,
    hardUpdate = 2,
    fullUpdate = 3,
    deadWindow = -1
} UpdateMode;

typedef enum Input{
    B0 = BIT0,
    B1 = BIT1,
    B2 = BIT2,  // INPUT 1
    B3 = BIT3,  // INPUT 1
    B4 = BIT4,
    TOUCH = BIT5
} Input;


//*** Change the flagging and focus system!!
typedef struct Window{
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
    uint8_t layer;
    void(*Render)(void);
    void(*OnTouch)(void);
    uint16_t trackingDelay;
    struct Window* next;
    struct Window* prev;
    UpdateMode update;
    bool visible;
    bool remove;
    uint8_t inputField;
    bool inputFlag;         // so you only have to check one
    Point touchPoint;       // {-1, -1} by default
    struct Button* buttonHead;
    uint8_t buttonCount;
    ID_t ID;
} Window;

typedef struct Button{
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
    uint16_t bgColor;
    uint16_t outlineColor;
    uint16_t fontColor;
    uint8_t fontSize;
    uint8_t outlineThickness;
    char* text;
    uint8_t textLength;
    void(*Render)(struct Button* button);
    void(*OnPress)(void);
    struct Button* next;
    struct Button* prev;
    Window* parent;
    UpdateMode update;      // if update != hardUpdate when the render function is called, the button has been pressed or released.
    bool pressed;
    bool visible;
    bool remove;
    ID_t ID;
} Button;


#endif /* REDLIB_INC_REDTYPES_H_ */
