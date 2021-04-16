/*
 * HostMenu.c
 *
 *  Created on: Apr 28, 2020
 *      Author: tyvanroy
 */


#include "HostMenu.h"
#include "RedLib.h"
#include "SpawnPlane.h"
#include "World.h"
#include "RedstoneHost.h"
#include "SliderWindow.h"
#include <string.h>

#define TITLE                   "3D Server"
#define SUBTITLE                "..."

#define TITLE_SPACE                 40
#define TOP_PADDING                 7
#define LEFT_PADDING                7

#define BUTTON_HEIGHT               30

#define DELETE_OBJECTS_BUTTON_WIDTH     150
#define DELETE_OBJECTS_BUTTON_HEIGHT    BUTTON_HEIGHT
#define RESET_PLAYER_BUTTON_WIDTH       150
#define RESET_PLAYER_BUTTON_HEIGHT      BUTTON_HEIGHT
#define SPAWN_OBJECTS_BUTTON_WIDTH      150
#define SPAWN_OBJECTS_BUTTON_HEIGHT     BUTTON_HEIGHT
#define TOGGLE_RENDER_BUTTON_WIDTH      150
#define TOGGLE_RENDER_BUTTON_HEIGHT     BUTTON_HEIGHT
#define LAUNCH_SERVER_BUTTON_WIDTH      150
#define LAUNCH_SERVER_BUTTON_HEIGHT     BUTTON_HEIGHT

#define PADDING                 10
#define LABEL_HEIGHT            20

#define SLIDER_WINDOW_WIDTH     ((MAX_SCREEN_X >> 1) - PADDING)
#define SLIDER_WINDOW_HEIGHT    (BUTTON_HEIGHT + LABEL_HEIGHT)

#define EDIT_WINDOW_X       (MAX_SCREEN_X >> 1)
#define EDIT_WINDOW_Y       (TITLE_SPACE + TOP_PADDING)
#define EDIT_WINDOW_WIDTH   (MAX_SCREEN_X >> 1)
#define EDIT_WINDOW_HEIGHT  (MAX_SCREEN_Y - EDIT_WINDOW_Y - SLIDER_WINDOW_HEIGHT)

#define SLIDER_X    (MAX_SCREEN_X >> 1)
#define SLIDER_Y    (MAX_SCREEN_Y - SLIDER_WINDOW_HEIGHT)
#define SPAWN_PLANE_X   ((MAX_SCREEN_X >> 1) + 6)
#define SPAWN_PLANE_Y   (TITLE_SPACE)
#define SPAWN_PLANE_WIDTH   ((MAX_SCREEN_X >> 1) - 6)
#define SPAWN_PLANE_HEIGHT  SPAWN_PLANE_WIDTH


static Poly Cube = {
                    8,
                    {{-0.5,-0.5, -0.5},
                     {0.5, -0.5, -0.5},
                     {0.5, 0.5, -0.5},
                     {-0.5, 0.5, -0.5},
                     {-0.5,-0.5, 0.5},
                     {0.5, -0.5, 0.5},
                     {0.5, 0.5, 0.5},
                     {-0.5, 0.5, 0.5}},
                    12,
                    {{0, 1},
                     {1, 2},
                     {2, 3},
                     {3, 0},
                     {4, 5},
                     {5, 6},
                     {6, 7},
                     {7, 4},
                     {0, 4},
                     {1, 5},
                     {2, 6},
                     {3, 7}}
};

static Button* LaunchServerButton;
static Button* DeleteObjectsButton;
static Button* ResetPlayerButton;
static Button* SpawnObjectButton;
static Button* ToggleRenderButton;

typedef enum HostStatus{
    notHosting,
    waiting,
    deployed
} HostStatus;

static HostStatus status;

void InitHostMenu(void){

    SpawnDepth = 50;
    SpawnScale = 5;
    SpawnxVel = 0;
    SpawnyVel = 0;
    SpawnzVel = 0;
    SpawnxRot = 0;
    SpawnyRot = 0;
    SpawnzRot = 0;

    status = notHosting;

    HostMenu = GetWindow(AddWindow(0, 0, MAX_SCREEN_X, MAX_SCREEN_Y, TOP_LAYER - 1, RenderHostMenu));
    InitSliderWindow(SLIDER_X, SLIDER_Y, SLIDER_WINDOW_WIDTH, SLIDER_WINDOW_HEIGHT);

    AddSlider("zPosition", &SpawnDepth, -100, 100);
    AddSlider("Scale", &SpawnScale, 1, 100);
    AddSlider("xVelocity", &SpawnxVel, -100, 100);
    AddSlider("yVelocity", &SpawnyVel, -100, 100);
    AddSlider("zVelocity", &SpawnzVel, -100, 100);
    AddSlider("xRotVel", &SpawnxRot, -10, 10);
    AddSlider("yRotVel", &SpawnyRot, -10, 10);
    AddSlider("zRotVel", &SpawnzRot, -10, 10);

    InitSpawnPlane(SPAWN_PLANE_X, SPAWN_PLANE_Y, SPAWN_PLANE_WIDTH, SPAWN_PLANE_HEIGHT);

    LaunchServerButton = GetButton(AddButton(LEFT_PADDING, TOP_PADDING + TITLE_SPACE, LAUNCH_SERVER_BUTTON_WIDTH, LAUNCH_SERVER_BUTTON_HEIGHT, LaunchSeverButtonOnPress, HostMenu));
    SetButtonText(LaunchServerButton, "Launch Server");
    SetButtonOutlineThickness(LaunchServerButton, 3);
    SetButtonOutlineColor(LaunchServerButton, LCD_BLUE);

    DeleteObjectsButton = GetButton(AddButton(LEFT_PADDING, TOP_PADDING + TITLE_SPACE + LAUNCH_SERVER_BUTTON_HEIGHT, DELETE_OBJECTS_BUTTON_WIDTH, DELETE_OBJECTS_BUTTON_HEIGHT, DeleteObjectsButtonOnPress, HostMenu));
    SetButtonText(DeleteObjectsButton, "Delete Objects");
    SetButtonOutlineThickness(DeleteObjectsButton, 3);
    SetButtonOutlineColor(DeleteObjectsButton, LCD_RED);

    ResetPlayerButton = GetButton(AddButton(LEFT_PADDING, TOP_PADDING + TITLE_SPACE + LAUNCH_SERVER_BUTTON_HEIGHT + DELETE_OBJECTS_BUTTON_HEIGHT, RESET_PLAYER_BUTTON_WIDTH, RESET_PLAYER_BUTTON_HEIGHT, ResetPlayerButtonOnPress, HostMenu));
    SetButtonText(ResetPlayerButton, "Reset Player");
    SetButtonOutlineThickness(ResetPlayerButton, 3);
    SetButtonOutlineColor(ResetPlayerButton, LCD_MAGENTA);

//    SpawnObjectButton = GetButton(AddButton(LEFT_PADDING, TOP_PADDING + TITLE_SPACE + LAUNCH_SERVER_BUTTON_HEIGHT + DELETE_OBJECTS_BUTTON_HEIGHT + RESET_PLAYER_BUTTON_HEIGHT, SPAWN_OBJECTS_BUTTON_WIDTH, SPAWN_OBJECTS_BUTTON_HEIGHT, SpawnObjectButtonOnPress, HostMenu));
//    SetButtonText(SpawnObjectButton, "Spawn Object");
//    SetButtonOutlineThickness(SpawnObjectButton, 2);
//    SetButtonOutlineColor(SpawnObjectButton, LCD_CYAN);

    ToggleRenderButton = GetButton(AddButton(LEFT_PADDING,TOP_PADDING + TITLE_SPACE + LAUNCH_SERVER_BUTTON_HEIGHT + DELETE_OBJECTS_BUTTON_HEIGHT + RESET_PLAYER_BUTTON_HEIGHT, TOGGLE_RENDER_BUTTON_WIDTH, TOGGLE_RENDER_BUTTON_HEIGHT, ToggleRenderButtonOnPress, HostMenu));
    SetButtonText(ToggleRenderButton, "Toggle Render Mode");
    SetButtonOutlineThickness(ToggleRenderButton, 2);
    SetButtonOutlineColor(ToggleRenderButton, LCD_CYAN);

    SetVisible(HostMenu, true);
    GiveFocus(HostMenu);
}

void RenderHostMenu(void){
    if(Context->update == hardUpdate){
        Write((MAX_SCREEN_X >> 1) - (strlen(TITLE) << 3), TOP_PADDING, TITLE, 2, LCD_RED);
        Write((MAX_SCREEN_X >> 1) - (strlen(SUBTITLE) << 2), TOP_PADDING + 20, SUBTITLE, 1, LCD_WHITE);
    }
}

void LaunchSeverButtonOnPress(void){
    while(InputPressed(TOUCH)){
        sleep(1);
    }
    LaunchServerButton->pressed = false;

    if(status == notHosting){
        sleep(15);
        LaunchRedstoneHost();
    }else{
        sleep(15);
    }
    FinishOnPress();
}

void SetToWaiting(void){
    status = waiting;
    SetButtonText(LaunchServerButton, "Waiting For Client");
    SetButtonOutlineColor(ToggleRenderButton, LCD_YELLOW);
    Flag(LaunchServerButton);                // flag for rendering
}

void SetToDeployed(void){
    status = deployed;
    SetButtonText(LaunchServerButton, "Deployed!");
    SetButtonOutlineColor(LaunchServerButton, LCD_GREEN);
    Flag(LaunchServerButton);                // flag for rendering
}

void DeleteObjectsButtonOnPress(void){
    while(InputPressed(TOUCH)){
        sleep(1);
    }
    DeleteObjectsButton->pressed = false;
    Flag(DeleteObjectsButton);                // flag for rendering

    sleep(15);

    RemoveAllWorldObjects();

    FinishOnPress();
}

void ResetPlayerButtonOnPress(void){
    while(InputPressed(TOUCH)){
        sleep(1);
    }
    ResetPlayerButton->pressed = false;
    Flag(ResetPlayerButton);                // flag for rendering

    sleep(15);

    ResetPlayer();

    FinishOnPress();
}

void SpawnObjectButtonOnPress(void){
    while(InputPressed(TOUCH)){
        sleep(1);
    }
    SpawnObjectButton->pressed = false;
    Flag(SpawnObjectButton);                // flag for rendering

    sleep(15);

    Point3 position = {0, 0, (-(SpawnDepth * 10)) + 25};
    Point3 pVelocity = {SpawnxVel, SpawnyVel, SpawnzVel};
    float pp = (PI * .0005555556);
    Point3 rVelocity = {pp * ((float) SpawnxRot), pp * ((float) SpawnyRot), pp * ((float) SpawnzRot)};
    AddWorldObject(position, pVelocity, rVelocity, &Cube, SpawnScale, LCD_MAGENTA);

    FinishOnPress();
}

void ToggleRenderButtonOnPress(void){
    while(InputPressed(TOUCH)){
        sleep(1);
    }
    ToggleRenderButton->pressed = false;
    Flag(ToggleRenderButton);                // flag for rendering

    sleep(15);

    ToggleRenderMode();

    FinishOnPress();
}

