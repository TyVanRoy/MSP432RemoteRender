/*
 * DemoApp.c
 *
 *  Created on: Feb 17, 2020
 *      Author: tyvanroy
 */
#include <DemoApp.h>
#include <string.h>

static const Point END = {MAX_SCREEN_X, MAX_SCREEN_Y};
static const Point CENTER = {MAX_SCREEN_X >> 1, MAX_SCREEN_Y >> 1};

static Page ActivePage;
static semaphore_t RenderLock;

/* Windows */
static Window* MainMenu;
static Window* FreeDrawWindow;
static Window* LineDrawWindow;
static Window* ThreeDWindow;
static Window* GyroSketchWindow;

/* MainMenu Variables */
static const char* HEADER_TITLE = "Red Library Demo";
static const uint8_t HEADER_HEIGHT = 60;

static Button* BadDrawButton;
static Button* GyroSketchButton;
static Button* ThreeDDemoButton;
static Button* InfoPageButton;

/* BadDraw Variables */
static bool drawing = false;
static Point startPoint;
static Point oldPoint;

/* GyroSketch Variables */
static const int GYRO_SIZE = 100;
static const int GYRO_THICKNESS = 1;
typedef struct Ellipse{
    Point center;
    uint16_t width;
    uint16_t height;
} Ellipse;
static Ellipse outer;
static Ellipse xring;
static Ellipse yring;
static Ellipse oxring;
static Ellipse oyring;
static int16_t accX;
static int16_t accY;
static int xd, yd;

/* 3D Variables */
static Camera cam = {(Point3) {MAX_SCREEN_X >> 1, MAX_SCREEN_Y >> 1}, (Point3) {0, 0}, };
typedef enum ThreeDMode{
    rotate,
    zforward,
    zbackward
} ThreeDMode;
static bool modeChange = true;
static bool updateFlag = true;
static float maxTheta = (2.0 * PI) / 36;
static float maxDT = 30;
static float theta = 0;
static float dt = 0;
static float xRot = 0;
static float yRot = 0;
static float zRot = 0;
static Point3 translate = {0, 0, 0};
static ThreeDMode mode = rotate;

static Point3 cold[8] =
                    {{-25,-25, 650},
                     {25, -25, 650},
                     {25, 25, 650},
                     {-25, 25, 650},
                     {-25,-25, 700},
                     {25, -25, 700},
                     {25, 25, 700},
                     {-25, 25, 700}};
static Point3 cnew[8];

void GotoPage(Page page){
    switch(page){
        case MainMenuPage:
            SetVisible(ThreeDWindow, false);
            SetVisible(GyroSketchWindow, false);
            SetVisible(FreeDrawWindow, false);
            SetVisible(LineDrawWindow, false);
            SetVisible(MainMenu, true);

            GiveFocus(&DefaultContext);
            break;
        case BadDraw:
            SetVisible(MainMenu, false);
            SetVisible(ThreeDWindow, false);
            SetVisible(GyroSketchWindow, false);
            SetVisible(FreeDrawWindow, true);
            SetVisible(LineDrawWindow, true);
            break;
        case GyroSketch:
            G8RTOS_InitSemaphore(&RenderLock, 1);
            G8RTOS_AddThread(GyroSketchThread, 2, "gyrosketch");

            SetVisible(MainMenu, false);
            SetVisible(ThreeDWindow, false);
            SetVisible(FreeDrawWindow, false);
            SetVisible(LineDrawWindow, false);
            SetVisible(GyroSketchWindow, true);

            GiveFocus(GyroSketchWindow);
            break;
        case ThreeDDemo:
            G8RTOS_InitSemaphore(&RenderLock, 1);
            ClonePoints(cnew, cold, 8, 1);
            SetGlobalCam(&cam, false);
            G8RTOS_AddThread(ThreeDDemoThread, 2, "3d demo");

            SetVisible(MainMenu, false);
            SetVisible(GyroSketchWindow, false);
            SetVisible(FreeDrawWindow, false);
            SetVisible(LineDrawWindow, false);
            SetVisible(ThreeDWindow, true);

            GiveFocus(ThreeDWindow);
            break;
        case InfoPage:
            break;
    }

    ActivePage = page;
}

void DemoAppMain(void){

    /* Main Menu */
    MainMenu = GetWindow(AddWindow(0, 0, END.x, END.y, TOP_LAYER, RenderDemoMainMenu));

    int buttonWidth = MAX_SCREEN_X >> 1;
    int buttonHeight = (MAX_SCREEN_Y - HEADER_HEIGHT) >> 1;

    BadDrawButton = GetButton(AddButton(0, HEADER_HEIGHT, buttonWidth, buttonHeight, BadDrawButtonOnPress, MainMenu));
    SetButtonText(BadDrawButton, "Bad Drawing");
    SetButtonOutlineThickness(BadDrawButton, 3);
    SetButtonOutlineColor(BadDrawButton, LCD_ORANGE);

    GyroSketchButton = GetButton(AddButton(buttonWidth, HEADER_HEIGHT, buttonWidth, buttonHeight, GyroSketchButtonOnPress, MainMenu));
    SetButtonText(GyroSketchButton, "GyroSketch");
    SetButtonOutlineThickness(GyroSketchButton, 3);
    SetButtonOutlineColor(GyroSketchButton, LCD_OLIVE);

    ThreeDDemoButton = GetButton(AddButton(0, HEADER_HEIGHT + buttonHeight, buttonWidth, buttonHeight, ThreeDDemoButtonOnPress, MainMenu));
    SetButtonText(ThreeDDemoButton, "3D Demo");
    SetButtonOutlineThickness(ThreeDDemoButton, 3);
    SetButtonOutlineColor(ThreeDDemoButton, LCD_MAGENTA);

    InfoPageButton = GetButton(AddButton(buttonWidth, HEADER_HEIGHT + buttonHeight, buttonWidth, buttonHeight, InfoPageButtonOnPress, MainMenu));
    SetButtonText(InfoPageButton, "Info!");
    SetButtonOutlineThickness(InfoPageButton, 3);
    SetButtonOutlineColor(InfoPageButton, LCD_BLUE);


    /* Bad Draw Page */
    FreeDrawWindow = GetWindow(AddWindow(0, 0, END.x >> 1, END.y, BOTTOM_LAYER, RenderFreeDrawWindow));
    LineDrawWindow = GetWindow(AddWindow(END.x >> 1, 0, END.x >> 1, END.y, BOTTOM_LAYER, RenderLineDrawWindow));

    SetTrackingDelay(FreeDrawWindow, 1);

    // Right window gets the line-draw, left window uses DefaultOnTouch for free-hand drawing.
    SetOnTouchHandler(LineDrawWindow, LineDrawOnTouch);


    /* 3D Page */
    ThreeDWindow = GetWindow(AddWindow(0, 0, END.x, END.y, BOTTOM_LAYER, Render3DWindow));

    /* GyroSketch Page */
    GyroSketchWindow = GetWindow(AddWindow(0, 0, END.x, END.y, BOTTOM_LAYER, RenderGyroSketchWindow));


    /* Main program */

    GotoPage(MainMenuPage);
    while(1){

        switch(ActivePage){
            case BadDraw:
                if(FreeDrawWindow->inputFlag){
                    SoftUpdate(FreeDrawWindow);
                }
            break;
        }

        if(ActivePage != MainMenuPage){
            if(FocusWindow->inputField & B1){
                FocusWindow->inputField &= ~(B1);

                sleep(10);
                UnmaskInput(B1);
                GotoPage(MainMenuPage);
            }
        }

        sleep(50);
    }
}

void ThreeDDemoThread(void){

    ClonePoints(cnew, cold, 8, 1);
    Joystick_Init_Without_Interrupt();

    static Point3 origin = {0, 0, 675};
    int threshold = 1000;
    int16_t xc;
    int16_t yc;
    while(ActivePage == ThreeDDemo){

        xc = 0;
        yc = 0;
        GetJoystickCoordinates(&xc, &yc);

        if(abs(xc) > threshold){

            if(mode == rotate){
                theta = maxTheta - (maxTheta / (abs(xc) / 750));
                yRot = xc < 0 ? theta : -theta;
            }else{
                dt = maxDT - (maxDT / (abs(xc) / 750));
                translate.x = xc < 0 ? dt : -dt;
            }

            updateFlag = true;
        }
        if(abs(yc) > threshold){

            if(mode == rotate){
                theta = maxTheta - (maxTheta / (abs(yc) / 750));
                xRot = yc < 0 ? -theta : theta;
            }else{
                dt = maxDT - (maxDT / (abs(yc) / 750));
                translate.y = yc < 0 ? -dt : dt;
            }

            updateFlag = true;
        }

        if(ThreeDWindow->inputFlag){
            if(ThreeDWindow->inputField & B0){
                sleep(10);  // debounce

                switch(mode){
                    case rotate:
                        mode = zforward;
                        break;
                    case zforward:
                        mode = zbackward;
                        break;
                    case zbackward:
                        mode = rotate;
                        break;
                }

                ThreeDWindow->inputField &= ~(B0);
                UnmaskInput(B0);
                modeChange = true;
                SoftUpdate(ThreeDWindow);
            }
            if(ThreeDWindow->inputField & B4){
                switch(mode){
                    case rotate:
                        zRot = maxTheta / 3;
                        break;
                    case zforward:
                        translate.z = 10;
                        break;
                    case zbackward:
                        translate.z = -10;
                        break;
                }
                updateFlag = true;

            }else{
                ThreeDWindow->inputField &= ~(B4);
            }
        }

        if(!InputPressed(B4) && (ThreeDWindow->inputField & B4)){
            ThreeDWindow->inputFlag = false;
            ThreeDWindow->inputField &= ~(B4);
            UnmaskInput(B4);
        }

        if(updateFlag){

            SemWait(&RenderLock);
            Point3 rot = {xRot, yRot, zRot};
            RotatePoints(cnew, 8, origin, rot);
            xRot = 0;
            yRot = 0;
            zRot = 0;

            if(translate.x != 0 || translate.y != 0 || translate.z != 0){
                TranslatePoints(cnew, 8, translate);
                origin.x += translate.x;
                origin.y += translate.y;
                origin.z += translate.z;
                translate.x = 0;
                translate.y = 0;
                translate.z = 0;
            }

            updateFlag = false;
            SemSignal(&RenderLock);
            SoftUpdate(ThreeDWindow);
        }

        sleep(20);
    }

    G8RTOS_KillSelf();
}

void GyroSketchThread(void){
    outer.center = CENTER;
    outer.height = GYRO_SIZE;
    outer.width = GYRO_SIZE;

    xring.center = CENTER;
    xring.width = GYRO_SIZE - 10;
    xring.height = GYRO_SIZE - 10;

    yring.center = CENTER;
    yring.width = GYRO_SIZE - 10;
    yring.height = GYRO_SIZE - 10;

    oxring.center = CENTER;
    oxring.width = GYRO_SIZE - 10;
    oxring.height = GYRO_SIZE - 10;

    oyring.center = CENTER;
    oyring.width = GYRO_SIZE - 10;
    oyring.height = GYRO_SIZE - 10;

    while(ActivePage == GyroSketch){

        while(bmi160_read_accel_x(&accX));        // read accelerometer
        while(bmi160_read_accel_y(&accY));        // read accelerometer

        SemWait(&RenderLock);
        accX = accX < 0 ? -accX : accX;
        accY = accY < 0 ? -accY : accY;
        SemSignal(&RenderLock);

        SoftUpdate(GyroSketchWindow);

        sleep(25);
    }

    G8RTOS_KillSelf();
}

/** Touch Functions **/

void LineDrawOnTouch(void){
    Point tp = {0, 0};
    TP_ReadXY(&tp);
    ScaleTP(&tp);

    startPoint.x = tp.x;
    startPoint.y = tp.y;
    oldPoint.x = startPoint.x;
    oldPoint.y = startPoint.y;
    drawing = true;

    while(InputPressed(TOUCH)){

        TP_ReadXY(&tp);
        ScaleTP(&tp);

        FocusWindow->touchPoint.x = tp.x;
        FocusWindow->touchPoint.y = tp.y;
        SoftUpdate(LineDrawWindow);

        sleep(FocusWindow->trackingDelay);
    }

    drawing = false;
    FinishOnTouch();
}

/** Render Functions **/

void RenderDemoMainMenu(void){

    if(Context->update == hardUpdate){
        int size = 2;
        int offset = 0;
        int titleLength = strlen(HEADER_TITLE) * size;

        int tx = ((Context->width >> 1)) - (titleLength << 2);
        int ty = (HEADER_HEIGHT >> 1) - ((16 * size) >> 1) + offset;

        Write(tx, ty, (char*) HEADER_TITLE, size, LCD_WHITE);
    }

}

void RenderGyroSketchWindow(void){
    if(Context->update == hardUpdate){
        for(int i = 0; i < 20; i++){
            DrawEllipse(outer.center.x, outer.center.y, outer.width, outer.height, 1, 0xffff);
            outer.width = i % 2 == 0 ? outer.width + 1 : outer.width;
            outer.height = i % 2 != 0 ? outer.height + 1 : outer.height;
        }

        return;
    }

    SemWait(&RenderLock);
    accX /= 1000;
    accY /= 1000;
    xd = accX - 1;
    yd = accY - 1;

    oxring.width = xring.width;
    oyring.height = yring.height;
    xring.width = GYRO_SIZE - 10 - (((GYRO_SIZE - 10) >> 4) * xd);
    yring.height = GYRO_SIZE - 10 - (((GYRO_SIZE - 10) >> 4) * yd);

    if(oxring.width != xring.width){
       DrawEllipse(xring.center.x, xring.center.y, xring.width, xring.height, GYRO_THICKNESS, LCD_MAGENTA);
       DrawEllipse(oxring.center.x, oxring.center.y, oxring.width, oxring.height, GYRO_THICKNESS, LCD_BLACK);
    }

    if(oyring.height != yring.height){
       DrawEllipse(yring.center.x, yring.center.y, yring.width, yring.height, GYRO_THICKNESS, LCD_GREEN);
       DrawEllipse(oyring.center.x, oyring.center.y, oyring.width, oyring.height, GYRO_THICKNESS, LCD_BLACK);
    }
    SemSignal(&RenderLock);
}

void RenderFreeDrawWindow(void){

    // hard update
    if(Context->update == hardUpdate){
        FillRect(0, 0, Context->width, Context->height, LCD_BLACK);
        DrawRect(0, 0, Context->width, Context->height, 3, LCD_MAGENTA);
        DrawLine(0, 20, Context->width, 20, 3, LCD_MAGENTA);

        Write(Context->width / 2 - (9 * 8 / 2), 3, "Free Draw", 1, LCD_WHITE);

        return;
    }

    int size = 4;

    if(Context->inputField & TOUCH){
        FillRect(Context->touchPoint.x - (size >> 1), Context->touchPoint.y - (size >> 1), size, size, LCD_WHITE);
    }

}

void RenderLineDrawWindow(void){
    // hard
    if(Context->update == hardUpdate){
        FillRect(0, 0, Context->width, Context->height, LCD_BLACK);
        DrawRect(0, 0, Context->width, Context->height, 3, LCD_CYAN);
        DrawLine(0, 20, Context->width, 20, 3, LCD_CYAN);

        Write(Context->width / 2 - (9 * 8 / 2), 3, "Line Draw", 1, LCD_WHITE);

        return;
    }

    // soft
    if(drawing){
        DrawLine(startPoint.x, startPoint.y, oldPoint.x, oldPoint.y, 5, LCD_BLACK);
        DrawLine(startPoint.x, startPoint.y, Context->touchPoint.x, Context->touchPoint.y, 2, LCD_WHITE);

        oldPoint.x = Context->touchPoint.x;
        oldPoint.y = Context->touchPoint.y;
    }
}

void Render3DWindow(void){
    if(Context->update == hardUpdate){
        FillRect(0, 0, Context->width, Context->height, LCD_WHITE);
        DrawRect(0, 0, Context->width, Context->height, 3, LCD_MAGENTA);
        DrawLine(0, 20, Context->width, 20, 3, LCD_MAGENTA);

        Write(Context->width / 2 - (17 * 8 / 2), 3, "Red3D Engine Demo", 1, LCD_BLACK);

        char* line1 = "Mode toggle - B0";
        char* line2 = "Back - B1";
        int line1Len = strlen(line1);
        int line2Len = strlen(line2);

        int rightPadding = 8;
        int bottomPadding = 8;
        int tweenPadding = 2;
        int x1 = MAX_SCREEN_X - rightPadding - (line1Len * 8);
        int x2 = MAX_SCREEN_X - rightPadding - (line2Len * 8);
        int y1 = MAX_SCREEN_Y - bottomPadding - (16 * 2) - tweenPadding;
        int y2 = y1 + 16 + tweenPadding;

        Write(x1, y1, line1, 1, LCD_BLACK);
        Write(x2, y2, line2, 1, LCD_BLACK);
    }

    if(modeChange){
        char* modeText;
        switch(mode){
            case rotate:
                modeText = "Rotate Mode";
                break;
            case zforward:
                modeText = "Translate Mode -zoom in";
                break;
            case zbackward:
                modeText = "Translate Mode -zoom out";

        }

        int x = 8;
        int y = MAX_SCREEN_Y - 24;

        ClearText(x, y, 25, LCD_WHITE);
        Write(x, y, modeText, 1, LCD_BLACK);

        modeChange = false;
    }

    SemWait(&RenderLock);
    DrawPrism(cnew, 1, 0x0000);
    DrawPrism(cold, 1, 0xffff);
    DrawPrism(cnew, 1, 0x0000);

    ClonePoints(cold, cnew, 8, 1);
    SemSignal(&RenderLock);
}


/** Button Press Functions **/

void BadDrawButtonOnPress(void){
    while(InputPressed(TOUCH)){
        sleep(1);
    }
    BadDrawButton->pressed = false;
    Flag(BadDrawButton);                // flag for rendering

    sleep(15);
    GotoPage(BadDraw);

    FinishOnPress();
}

void GyroSketchButtonOnPress(void){
    while(InputPressed(TOUCH)){
        sleep(1);
    }
    GyroSketchButton->pressed = false;
    Flag(GyroSketchButton);                // flag for rendering

    sleep(15);
    GotoPage(GyroSketch);

    FinishOnPress();
}

void ThreeDDemoButtonOnPress(void){
    while(InputPressed(TOUCH)){
        sleep(1);
    }
    ThreeDDemoButton->pressed = false;
    Flag(ThreeDDemoButton);             // flag for rendering

    sleep(15);
    GotoPage(ThreeDDemo);

    FinishOnPress();
}

void InfoPageButtonOnPress(void){
    while(InputPressed(TOUCH)){
        sleep(1);
    }
    InfoPageButton->pressed = false;
    Flag(InfoPageButton);               // flag for rendering

    sleep(15);
    GotoPage(InfoPage);

    FinishOnPress();
}


/** Init functions **/
void DemoAppLaunch(void){
    G8RTOS_AddThread(DemoAppMain, 0xAA, "demoappmain");
}

