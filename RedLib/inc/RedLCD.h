/*
 * RedLCD.h
 *
 *  Created on: Mar 6, 2020
 *      Author: tyvanroy
 */

#ifndef REDLIB_REDLCD_H_
#define REDLIB_REDLCD_H_

#include <RedTypes.h>

/**
 * Everything in this module is translated based on the Context Window's coordinates (Not scaled, however).
 *  i.e. x and y coordinates are translated by Context.x and Context.y passed to LCD_Rect and LCD_PutPoint.
 *
 *   Note: The WindowManager will change this to the Window-being-rendered's Render function before calling it.
 *
 * **/
Window* Context;

/** Call before using this module **/
void InitRedLCD(void);

inline void DrawPoint(uint16_t x, uint16_t y, uint16_t color);

/** Rectangles **/
inline bool DrawXLine(uint16_t x1, uint16_t x2, uint16_t y, uint16_t thickness, uint16_t color);
inline bool DrawYLine(uint16_t y1, uint16_t y2, uint16_t x, uint16_t thickness, uint16_t color);
inline bool FillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);

/** No fill **/
inline bool DrawRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t thickness, uint16_t color);

/** Lines **/
void DrawLineLow(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t thickness, uint16_t color, bool antiAliasing);
void DrawLineHigh(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t thickness, uint16_t color, bool antiAliasing);
void HandleLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t thickness, uint16_t color, bool antiAliasing);
inline void DrawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t thickness, uint16_t color);
inline void DrawAALine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t thickness, uint16_t color);

/** Polygon **/
inline void DrawPoly(Point* points, uint8_t nPoints, uint16_t thickness, uint16_t color);
void RotatePoly(Point* points, uint8_t nPoints, Point origin, float degrees);

/** Ellipse **/
void DrawEllipse(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t thickness, uint16_t color);

/** Text **/
void WriteChar(uint16_t x, uint16_t y, char c, uint8_t size, uint16_t color);
void Write(uint16_t x, uint16_t y, char* str, uint8_t size, uint16_t color);
void ClearText(uint16_t x, uint16_t y, uint8_t length, uint16_t color);
void ClearTextLines(uint16_t x, uint16_t y, uint8_t lines, uint8_t length, uint16_t color);

/**
 * TouchScreen Scaling
 *
 *  These will translate their coordinates by position of the FocusWindow
 **/
inline void ScaleTP(Point* point);
static inline uint16_t ScaleTPX(uint16_t tx);
static inline uint16_t ScaleTPY(uint16_t ty);

#endif /* REDLIB_REDLCD_H_ */
