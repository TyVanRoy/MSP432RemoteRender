/*
 * DemoApp.h
 *
 *  Created on: Feb 17, 2020
 *      Author: tyvanroy
 */

#ifndef DEMOAPP_H_
#define DEMOAPP_H_

#include "G8RTOS_Semaphores.h"
#include "RedLib.h"

typedef enum Page
{
    MainMenuPage,
    BadDraw,
    GyroSketch,
    ThreeDDemo,
    InfoPage
} Page;

void DemoAppInit(void);
void DemoAppLaunch(void);

/* Background Threads */
void DemoAppMain(void);             // Main App Thread
void ThreeDDemoThread(void);    // Deserves its own thread.
void GyroSketchThread(void);      // Deserves its own thread.

void GotoPage(Page page);

/* Rendering Functions */
void RenderDemoMainMenu(void);
void RenderFreeDrawWindow(void);
void RenderLineDrawWindow(void);
void RenderGyroSketchWindow(void);
void Render3DWindow(void);

/* OnTouch Functions */
void LineDrawOnTouch(void);

/* Button Press Functions */
void BadDrawButtonOnPress(void);
void GyroSketchButtonOnPress(void);
void ThreeDDemoButtonOnPress(void);
void InfoPageButtonOnPress(void);

#endif /* DEMOAPP_H_ */
