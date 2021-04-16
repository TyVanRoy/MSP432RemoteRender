/*
 * RedLCD.c
 *
 *  Created on: Mar 6, 2020
 *      Author: tyvanroy
 */

#include "LCDLib.h"
#include "RedUI.h"
#include "RedLCD.h"
#include <math.h>
#include <AsciiLib.h>

void InitRedLCD(void){
    DefaultContext.x = 0;
    DefaultContext.y = 0;
    DefaultContext.width = MAX_SCREEN_X;
    DefaultContext.height = MAX_SCREEN_Y;
    DefaultContext.layer = 0;
    DefaultContext.OnTouch = DefaultOnTouch;
    DefaultContext.trackingDelay = DEFAULT_TRACKING_DELAY;
    DefaultContext.next = &DefaultContext;
    DefaultContext.prev = &DefaultContext;
    DefaultContext.update = noUpdate;
    DefaultContext.visible = true;
    DefaultContext.remove = false;
    DefaultContext.inputFlag = false;
    DefaultContext.inputField = 0;

    DefaultContext.touchPoint.x = -1;
    DefaultContext.touchPoint.y = -1;
    DefaultContext.buttonCount = 0;
    DefaultContext.ID = 0;

    Context = (Window*) &DefaultContext;
    FocusWindow = Context;
}

inline void DrawPoint(uint16_t x, uint16_t y, uint16_t color){
    if(x < 0 || y < 0){
        return;
    }
    LCD_SetPoint(x + Context->x, y + Context->y, color);
}

/** Rectangles **/
inline bool FillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color){
    return LCD_DrawRectangle(x + Context->x, y + Context->y, width, height, color);
}

inline bool DrawXLine(uint16_t x1, uint16_t x2, uint16_t y, uint16_t thickness, uint16_t color){
    if(x2 <= x1){
        return false;
    }
    return FillRect(x1, y, x2 - x1, thickness, color);
}

inline bool DrawYLine(uint16_t y1, uint16_t y2, uint16_t x, uint16_t thickness, uint16_t color){
    if(y2 <= y1){
        return false;
    }
    return FillRect(x, y1, thickness, y2 - y1, color);
}

inline bool DrawRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t thickness, uint16_t color){
    bool withoutHitch = true;

    withoutHitch |= DrawXLine(x, x + width, y, thickness, color);
    withoutHitch |= DrawXLine(x, x + width, y + height - thickness, thickness, color);
    withoutHitch |= DrawYLine(y, y + height, x, thickness, color);
    withoutHitch |= DrawYLine(y, y + height, x + width - thickness, thickness, color);

    return withoutHitch;
}

void DrawLineLow(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t thickness, uint16_t color, bool antiAliasing){
    bool div = thickness > 1 && !antiAliasing;
    if(div){
        x1 /= thickness;
        y1 /= thickness;
        x2 /= thickness;
        y2 /= thickness;
    }

    int dx = ((int) x2) - x1;
    int dy = ((int) y2) - y1;
    int yi = 1;

    if(dy < 0){
        yi = -1;
        dy = -dy;
    }
    int D = (dy << 2) - dx;
    int y = y1;

    for(int x = x1; x < x2; x++){
        if(div){
            FillRect(x * thickness, y * thickness, thickness, thickness, color);
        }else{
            DrawPoint(x, y, color);
        }
        if(D > 0){
            y = y + yi;
            D = D - (dx << 2);
        }
        D = D + (dy << 2);
    }
}

void DrawLineHigh(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t thickness, uint16_t color, bool antiAliasing){
    bool div = thickness > 1 && !antiAliasing;
    if(div){
        x1 /= thickness;
        y1 /= thickness;
        x2 /= thickness;
        y2 /= thickness;
    }
    int dx = ((int) x2) - x1;
    int dy = ((int) y2) - y1;
    int xi = 1;

    if(dx < 0){
        xi = -1;
        dx = -dx;
    }
    int D = (dx << 2) - dy;
    int x = x1;

    for(int y = y1; y < y2; y++){
        if(div){
            FillRect(x * thickness, y * thickness, thickness, thickness, color);
        }else{
            DrawPoint(x, y, color);
        }
        if(D > 0){
            x = x + xi;
            D = D - (dy << 2);
        }
        D = D + (dx << 2);
    }
}

void HandleLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t thickness, uint16_t color, bool antiAliasing){

    int absY = ((int) y2) - y1;
    int absX = ((int) x2) - x1;
    absX = absX < 0 ? -absX : absX;
    absY = absY < 0 ? -absY : absY;

    if(absY < absX){
        if(x1 > x2){
            DrawLineLow(x2, y2, x1, y1, thickness, color, antiAliasing);
        }else{
            DrawLineLow(x1, y1, x2, y2, thickness, color, antiAliasing);
        }
    }else{
        if(y1 > y2){
            DrawLineHigh(x2, y2, x1, y1, thickness, color, antiAliasing);
        }else{
            DrawLineHigh(x1, y1, x2, y2, thickness, color, antiAliasing);
        }
    }
}

inline void DrawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t thickness, uint16_t color){
    HandleLine(x1, y1, x2, y2, thickness, color, false);
}

inline void DrawAALine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t thickness, uint16_t color){
    HandleLine(x1, y1, x2, y2, thickness, color, true);
}

inline void DrawPoly(Point* points, uint8_t nPoints, uint16_t thickness, uint16_t color){

    for(int i = 0; i < nPoints; i++){
        if(i < nPoints - 1){
            DrawLine(points[i].x, points[i].y, points[i + 1].x, points[i + 1].y, thickness, color);
        }else{
            DrawLine(points[i].x, points[i].y, points[0].x, points[0].y, thickness, color);
        }
    }
}

void RotatePoly(Point* points, uint8_t nPoints, Point origin, float degrees){

    if(degrees > (2* PI)){
        degrees = 2 * PI;
    }

    float cosine = cos(degrees);
    float sine = sin(degrees);

    for(int i = 0; i < nPoints; i++){

        float fx = (float) (points[i].x - origin.x);
        float fy = (float) (points[i].y - origin.y);

        points[i].x = (int) origin.x + (fx * cosine - fy * sine);
        points[i].y = (int) origin.y + (fx * sine + fy * cosine);

    }

}

void DrawEllipse(uint16_t xPos, uint16_t yPos, uint16_t width, uint16_t height, uint16_t thickness, uint16_t color){
    bool div = thickness > 1;

    int a = width >> 1;
    int b = height >> 1;
    int xc = xPos;
    int yc = yPos;

    if(div){
        a /= thickness;
        b /= thickness;
        xc /= thickness;
        yc /= thickness;
    }

    int a2 = a * a;
    int b2 = b * b;
    int fa2 = (a2 << 2);
    int x, y, sigma;
    int xcmx, xcpx, ycmy, ycpy;

    for(x = 0, y = b, sigma = (b2 << 1) + a2 * (1 - (b << 1)); (b2 * x) <= (a2 * y); x++){
        xcmx = xc - x;
        xcpx = xc + x;
        ycmy = yc - y;
        ycpy = yc + y;

        if(div){
            xcmx *= thickness;
            xcpx *= thickness;
            ycmy *= thickness;
            ycpy *= thickness;
            FillRect(xcpx, ycpy, thickness, thickness, color);
            FillRect(xcmx, ycpy, thickness, thickness, color);
            FillRect(xcpx, ycmy, thickness, thickness, color);
            FillRect(xcmx, ycmy, thickness, thickness, color);
        }else{
            DrawPoint(xcpx, ycpy, color);
            DrawPoint(xcmx, ycpy, color);
            DrawPoint(xcpx, ycmy, color);
            DrawPoint(xcmx, ycmy, color);
        }

        if(sigma >= 0){
            sigma += fa2 * (1 - y);
            y--;
        }
        sigma += b2 * ((x << 2) + 6);
    }

    int fb2 = (b2 << 2);
    for(x = a, y = 0, sigma = (a2 << 1) + b2 * (1 - (a << 1)); (a2 * y) <= (b2 * x); y++){
        xcmx = xc - x;
        xcpx = xc + x;
        ycmy = yc - y;
        ycpy = yc + y;

        if(div){
            xcmx *= thickness;
            xcpx *= thickness;
            ycmy *= thickness;
            ycpy *= thickness;
            FillRect(xcpx, ycpy, thickness, thickness, color);
            FillRect(xcmx, ycpy, thickness, thickness, color);
            FillRect(xcpx, ycmy, thickness, thickness, color);
            FillRect(xcmx, ycmy, thickness, thickness, color);
        }else{
            DrawPoint(xcpx, ycpy, color);
            DrawPoint(xcmx, ycpy, color);
            DrawPoint(xcpx, ycmy, color);
            DrawPoint(xcmx, ycmy, color);
        }

        if(sigma >= 0){
            sigma += fb2 * (1 - x);
            x--;
        }
        sigma += a2 * ((y << 2) + 6);
    }
}

inline void WriteChar(uint16_t x, uint16_t y, char c, uint8_t size, uint16_t color){
    uint16_t i, j;
    uint8_t buffer[16];
    uint8_t temp;

    GetASCIICode(buffer, c);  /* get font data */
    for(i = 0; i < 16; i++){
        temp = buffer[i];
        for(j = 0; j < 8; j++){
            if((temp >> 7 - j) & (0x01 == 0x01)){
                if(size > 1){
                    FillRect(x + (j * size), y + (i * size), size, size, color);
                }else{
                    DrawPoint(x + j, y + i, color);
                }
            }
        }
    }
}

void Write(uint16_t x, uint16_t y, char* str, uint8_t size, uint16_t color){
    uint8_t temp;

    do{
        temp = *str++;
        WriteChar(x, y, temp, size, color);
        if(x < MAX_SCREEN_X - (8 * size)){
            x += (8 * size);
        }else if (y < MAX_SCREEN_X - (16 * size)){
            x = 0;
            y += (16 * size);
        }else{
            x = 0;
            y = 0;
        }
    }
    while(*str != 0);
}

void ClearText(uint16_t x, uint16_t y, uint8_t length, uint16_t color){
    FillRect(x, y,length * 8, 16, color);
}

void ClearTextLines(uint16_t x, uint16_t y, uint8_t lines, uint8_t length, uint16_t color){
    FillRect(x, y,length * 8, 16 * lines, color);
}

inline void ScaleTP(Point* p){
    p->x = ScaleTPX(p->x);
    p->y = ScaleTPY(p->y);
}

/*
 * min ~96
 * max ~4032
 * resting ~4095 ±63
 */
static inline uint16_t ScaleTPX(uint16_t tx){
    return (uint16_t) ( (((float) tx) - 384.0) / (4032.0 - 384.0) * MAX_SCREEN_X) - FocusWindow->x;
}

/*
 * min ~224
 * max ~4032
 * resting ~0 ±96
 */
static inline uint16_t ScaleTPY(uint16_t ty){
    return (uint16_t) ( (((float) ty) - 255.0) / (4032.0 - 255.0) * MAX_SCREEN_Y) - FocusWindow->y;
}
