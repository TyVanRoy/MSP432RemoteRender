/*
 * Canvas.c
 *
 *  Created on: Apr 24, 2020
 *      Author: tyvanroy
 */

#include "Canvas.h"
#include "RedLib.h"
#include "RedstoneClient.h"
#include "G8RTOS_Semaphores.h"
#include <string.h>
#include <stdio.h>

#define CONTROL_DEBUG false
static char DebugBuf[16];

#define CENTER_X            (MAX_SCREEN_X >> 1)
#define CENTER_Y            (MAX_SCREEN_Y >> 1)

#define HIT_MARKER_COLOR        LCD_RED
#define HIT_MARKER_SIZE         20
#define HIT_MARKER_THICKNESS    2
#define HIT_MARKER_X1           (CENTER_X - 20)
#define HIT_MARKER_X2           (CENTER_X + 20)
#define HIT_MARKER_Y1           (CENTER_Y - 20)
#define HIT_MARKER_Y2           (CENTER_Y + 20)

#define LASER_COOLING_COLOR     LCD_BLUE
#define LASER_CHARGING_COLOR    LCD_YELLOW
#define LASER_CHARGED_COLOR     LCD_MAGENTA

#define LASER_CHARGED_SIZE      10
#define CROSS_HAIR_COLOR        LCD_WHITE
#define CROSS_HAIR_SIZE         1
#define CROSS_HAIR_SIZE2        (CROSS_HAIR_SIZE << 1)

#define BUTTON_DOWN_COLOR   LCD_WHITE
#define BUTTON_UP_COLOR     LCD_MAGENTA
#define POST_BUTTON_X       (MAX_SCREEN_X - 74)
#define POST_BUTTON_Y       (MAX_SCREEN_Y - 30)
#define POST_BUTTON_SIZE    (10)

#define POST_B0_X           POST_BUTTON_X
#define POST_B0_Y           (POST_BUTTON_Y - POST_BUTTON_SIZE)
#define POST_B1_X           (POST_BUTTON_X + POST_BUTTON_SIZE)
#define POST_B1_Y           (POST_BUTTON_Y)
#define POST_B2_X           POST_BUTTON_X
#define POST_B2_Y           (POST_BUTTON_Y + POST_BUTTON_SIZE)
#define POST_B3_X           (POST_BUTTON_X - POST_BUTTON_SIZE)
#define POST_B3_Y           (POST_BUTTON_Y)
#define POST_B4_X           (POST_BUTTON_X - (POST_BUTTON_SIZE << 1))
#define POST_B4_Y           (POST_BUTTON_Y)

#define XYZ_POSX    0
#define XYZ_POSY    0

static LaserInfo laser;
static LaserInfo prevLaser;

static Frame_t FrameA;
static Frame_t FrameB;
static bool newFrame;
static uint8_t last = 0;    // 0 A, 1 B
static semaphore_t FRAMELOCK;

static uint8_t oldInputField = 0;

static bool frame1 = true;

static int32_t x;
static int32_t y;
static int32_t z;

static int32_t ox;
static int32_t oy;
static int32_t oz;

static char xyzBuf1[8];
static char xyzBuf2[8];

void PushCanvasFrame(GameStatePacket_t* source){
    SemWait(&FRAMELOCK);

    laser.charge = source->laser.charge;
    laser.status = source->laser.status;

    if(laser.status == la_asleep){
        SemWait(&CONTROLLOCK);
        if(ControlPacket.hitAcknowledge || ControlPacket.missAcknowledge){
            ControlPacket.hitAcknowledge = false;
            ControlPacket.missAcknowledge = false;
        }
        SemSignal(&CONTROLLOCK);
    }

    if(last == 0){
        FrameB.lineCount = source->frame.lineCount;

        for(int i = 0; i < FrameB.lineCount; i++){
            if(i >= MAX_FRAME_LINES){
                break;
            }
            FrameB.lines[i].a = source->frame.lines[i].a;
            FrameB.lines[i].b = source->frame.lines[i].b;
            FrameB.lines[i].color = source->frame.lines[i].color;
        }
    }else{
        FrameA.lineCount = source->frame.lineCount;

        for(int i = 0; i < FrameA.lineCount; i++){
            if(i >= MAX_FRAME_LINES){
                break;
            }
            FrameA.lines[i].a = source->frame.lines[i].a;
            FrameA.lines[i].b = source->frame.lines[i].b;
            FrameA.lines[i].color = source->frame.lines[i].color;
        }
    }

    x = source->x;
    y = source->y;
    z = source->z;

    newFrame = true;
    SoftUpdate(Canvas);
    SemSignal(&FRAMELOCK);
}

void RenderCanvas(void){
    if(Context->update == hardUpdate){
        LCD_Clear(BGCOLOR);
    }

    if(CONTROL_DEBUG){
        SemWait(&CONTROLLOCK);

        ClearTextLines(0, 0, 5, 16, BGCOLOR);

        sprintf(DebugBuf, "xAccel:%d", ControlPacket.accelX);
        Write(0, 0, DebugBuf, 1, LCD_BLACK);
        sprintf(DebugBuf, "input:%d", ControlPacket.inputField);
        Write(0, 16, DebugBuf, 1, LCD_BLACK);
        sprintf(DebugBuf, "xJoy:%d", ControlPacket.joystickX);
        Write(0, 32, DebugBuf, 1, LCD_BLACK);
        sprintf(DebugBuf, "zAccel:%d", ControlPacket.accelZ);
        Write(0, 48, DebugBuf, 1, LCD_BLACK);
        sprintf(DebugBuf, "yJoy:%d", ControlPacket.joystickY);
        Write(0, 64, DebugBuf, 1, LCD_BLACK);

        SemSignal(&CONTROLLOCK);
    }

    /* Post buttons */
    if(frame1){
        DrawEllipse(POST_B0_X, POST_B0_Y, POST_BUTTON_SIZE, POST_BUTTON_SIZE, 1, BUTTON_UP_COLOR);
        DrawEllipse(POST_B1_X, POST_B1_Y, POST_BUTTON_SIZE, POST_BUTTON_SIZE, 1, BUTTON_UP_COLOR);
        DrawEllipse(POST_B2_X, POST_B2_Y, POST_BUTTON_SIZE, POST_BUTTON_SIZE, 1, BUTTON_UP_COLOR);
        DrawEllipse(POST_B3_X, POST_B3_Y, POST_BUTTON_SIZE, POST_BUTTON_SIZE, 1, BUTTON_UP_COLOR);

        Write(POST_B0_X - 20, POST_B0_Y - 20, "Shoot", 1, LCD_WHITE);
        Write(POST_B1_X + 10, POST_B1_Y - 8, "Thrust", 1, LCD_WHITE);
        Write(POST_B2_X - 24, POST_B2_Y + 8, "Center", 1, LCD_WHITE);
        Write(POST_B3_X - (48), POST_B3_Y - POST_BUTTON_SIZE, "Break", 1, LCD_WHITE);

    }else{
        uint8_t field = GetInputField();
        if(oldInputField != field){

            if((oldInputField & B0) != (field & B0)){
                if(field & B0){
                    DrawEllipse(POST_B0_X, POST_B0_Y, POST_BUTTON_SIZE, POST_BUTTON_SIZE, 1, BUTTON_DOWN_COLOR);
                }else{
                    DrawEllipse(POST_B0_X, POST_B0_Y, POST_BUTTON_SIZE, POST_BUTTON_SIZE, 1, BUTTON_UP_COLOR);
                }
            }else if((oldInputField & B1) != (field & B1)){
                if(field & B1){
                    DrawEllipse(POST_B1_X, POST_B1_Y, POST_BUTTON_SIZE, POST_BUTTON_SIZE, 1, BUTTON_DOWN_COLOR);
                }else{
                    DrawEllipse(POST_B1_X, POST_B1_Y, POST_BUTTON_SIZE, POST_BUTTON_SIZE, 1, BUTTON_UP_COLOR);
                }
            }else if((oldInputField & B2) != (field & B2)){
                if(field & B2){
                    DrawEllipse(POST_B2_X, POST_B2_Y, POST_BUTTON_SIZE, POST_BUTTON_SIZE, 1, BUTTON_DOWN_COLOR);
                }else{
                    DrawEllipse(POST_B2_X, POST_B2_Y, POST_BUTTON_SIZE, POST_BUTTON_SIZE, 1, BUTTON_UP_COLOR);
                }
            }else if((oldInputField & B3) != (field & B3)){
                if(field & B3){
                    DrawEllipse(POST_B3_X, POST_B3_Y, POST_BUTTON_SIZE, POST_BUTTON_SIZE, 1, BUTTON_DOWN_COLOR);
                }else{
                    DrawEllipse(POST_B3_X, POST_B3_Y, POST_BUTTON_SIZE, POST_BUTTON_SIZE, 1, BUTTON_UP_COLOR);
                }
            }

            oldInputField = field;
        }
    }


    if(newFrame){
        SemWait(&FRAMELOCK);

        if(laser.status == la_hit){

            SemWait(&CONTROLLOCK);
            ControlPacket.hitAcknowledge = true;
            SemSignal(&CONTROLLOCK);

            DrawLine(HIT_MARKER_X1, HIT_MARKER_Y1, HIT_MARKER_X2, HIT_MARKER_Y2, HIT_MARKER_THICKNESS, HIT_MARKER_COLOR);
            DrawLine(HIT_MARKER_X2, HIT_MARKER_Y1, HIT_MARKER_X1, HIT_MARKER_Y2, HIT_MARKER_THICKNESS, HIT_MARKER_COLOR);

        }else if(laser.status == la_miss){

            SemWait(&CONTROLLOCK);
            ControlPacket.missAcknowledge = true;
            SemSignal(&CONTROLLOCK);

        }

        if(prevLaser.status == la_hit){
            DrawLine(HIT_MARKER_X1, HIT_MARKER_Y1, HIT_MARKER_X2, HIT_MARKER_Y2, HIT_MARKER_THICKNESS, BGCOLOR);
            DrawLine(HIT_MARKER_X2, HIT_MARKER_Y1, HIT_MARKER_X1, HIT_MARKER_Y2, HIT_MARKER_THICKNESS, BGCOLOR);
        }else if(prevLaser.status == la_miss){

        }

        if(last == 0){
            for(int i = 0; i < FrameA.lineCount; i++){
                DrawLine(FrameA.lines[i].a.x, FrameA.lines[i].a.y, FrameA.lines[i].b.x, FrameA.lines[i].b.y, 1, BGCOLOR);
            }
            for(int i = 0; i < FrameB.lineCount; i++){
                DrawLine(FrameB.lines[i].a.x, FrameB.lines[i].a.y, FrameB.lines[i].b.x, FrameB.lines[i].b.y, 1, FrameB.lines[i].color);
            }
            last = 1;
        }else{
            for(int i = 0; i < FrameB.lineCount; i++){
                DrawLine(FrameB.lines[i].a.x, FrameB.lines[i].a.y, FrameB.lines[i].b.x, FrameB.lines[i].b.y, 1, BGCOLOR);
            }
            for(int i = 0; i < FrameA.lineCount; i++){
                DrawLine(FrameA.lines[i].a.x, FrameA.lines[i].a.y, FrameA.lines[i].b.x, FrameA.lines[i].b.y, 1, FrameA.lines[i].color);
            }
            last = 0;
        }

        FillRect((MAX_SCREEN_X >> 1) - CROSS_HAIR_SIZE, (MAX_SCREEN_Y >> 1) - CROSS_HAIR_SIZE, CROSS_HAIR_SIZE2, CROSS_HAIR_SIZE2, CROSS_HAIR_COLOR);

        prevLaser.status = laser.status;

        if(x != ox || frame1){
            sprintf(xyzBuf1, "x: %d", ox);
            sprintf(xyzBuf2, "x: %d", x);

            Write(XYZ_POSX, XYZ_POSY, xyzBuf1, 1, LCD_BLACK);
            Write(XYZ_POSX, XYZ_POSY, xyzBuf2, 1, LCD_WHITE);

            ox = x;
        }
        if(y != oy || frame1){
            sprintf(xyzBuf1, "y: %d", oy);
            sprintf(xyzBuf2, "y: %d", y);

            Write(XYZ_POSX, XYZ_POSY + 16, xyzBuf1, 1, LCD_BLACK);
            Write(XYZ_POSX, XYZ_POSY + 16, xyzBuf2, 1, LCD_WHITE);

            oy = y;
        }
        if(z != oz || frame1){
            sprintf(xyzBuf1, "z: %d", oz);
            sprintf(xyzBuf2, "z: %d", z);

            Write(XYZ_POSX, XYZ_POSY + 32, xyzBuf1, 1, LCD_BLACK);
            Write(XYZ_POSX, XYZ_POSY + 32, xyzBuf2, 1, LCD_WHITE);

            oz = z;
        }
        newFrame = false;
        SemSignal(&FRAMELOCK);

        if(frame1){
            frame1 = false;
        }
    }
}

void InitCanvas(void){
    G8RTOS_InitSemaphore(&FRAMELOCK, 1);
    Canvas = GetWindow(AddWindow(0, 0, MAX_SCREEN_X, MAX_SCREEN_Y, BOTTOM_LAYER, RenderCanvas));

    FrameA.lineCount = 0;
    FrameB.lineCount = 0;

    SetVisible(Canvas, true);
    GiveFocus(Canvas);
}
