/*
 * Canvas.h
 *
 *  Created on: Apr 24, 2020
 *      Author: tyvanroy
 */

#ifndef APP_CLIENT_INC_CANVAS_H_
#define APP_CLIENT_INC_CANVAS_H_

#include "Redstone.h"
#include "RedLib.h"
#include "LCDLib.h"

#define BGCOLOR LCD_BLACK

Window* Canvas;

void PushCanvasFrame(GameStatePacket_t* source);
void RenderCanvas(void);
void InitCanvas(void);

#endif /* APP_CLIENT_INC_CANVAS_H_ */
