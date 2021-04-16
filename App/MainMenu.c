/*
 * MainMenu.c
 *
 *  Created on: Apr 28, 2020
 *      Author: tyvanroy
 */

#include "MainMenu.h"
#include "HostMenu.h"
#include "RedLib.h"
#include "RedstoneClient.h"
#include <string.h>

#define TITLE                   "REDSTONE"
#define SUBTITLE                "by Ty Van Roy & Guillermo Canelon"

#define TITLE_SPACE                 40
#define TOP_PADDING                 7

#define BUTTON_HEIGHT               55

#define HOST_BUTTON_WIDTH       (MAX_SCREEN_X - 100)
#define HOST_BUTTON_HEIGHT      (BUTTON_HEIGHT)
#define CLIENT_BUTTON_WIDTH     (MAX_SCREEN_X - 100)
#define CLIENT_BUTTON_HEIGHT    (BUTTON_HEIGHT)

#define DEMO_BUTTON_WIDTH       (HOST_BUTTON_WIDTH >> 1)
#define DEMO_BUTTON_HEIGHT      (BUTTON_HEIGHT)
#define INFO_BUTTON_WIDTH       DEMO_BUTTON_WIDTH
#define INFO_BUTTON_HEIGHT      (BUTTON_HEIGHT)

static Button* HostButton;
static Button* ClientButton;
static Button* DemoButton;
static Button* InfoButton;

void InitMainMenu(void){
    MainMenu = GetWindow(AddWindow(0, 0, MAX_SCREEN_X, MAX_SCREEN_Y, TOP_LAYER, RenderMainMenu));

    HostButton = GetButton(AddButton((MAX_SCREEN_X >> 1) - (HOST_BUTTON_WIDTH >> 1), (TOP_PADDING << 1) + TITLE_SPACE, HOST_BUTTON_WIDTH, HOST_BUTTON_HEIGHT, HostButtonOnPress, MainMenu));
    SetButtonText(HostButton, "Host 3D Server");
    SetButtonOutlineThickness(HostButton, 3);
    SetButtonOutlineColor(HostButton, LCD_RED);

    ClientButton = GetButton(AddButton((MAX_SCREEN_X >> 1) - (CLIENT_BUTTON_WIDTH >> 1), (TOP_PADDING << 1) + TITLE_SPACE + HOST_BUTTON_HEIGHT, CLIENT_BUTTON_WIDTH, CLIENT_BUTTON_HEIGHT, ClientButtonOnPress, MainMenu));
    SetButtonText(ClientButton, "Join 3D Server");
    SetButtonOutlineThickness(ClientButton, 3);
    SetButtonOutlineColor(ClientButton, LCD_MAGENTA);

    DemoButton = GetButton(AddButton((MAX_SCREEN_X >> 1) - (DEMO_BUTTON_WIDTH), (TOP_PADDING << 1) + TITLE_SPACE + HOST_BUTTON_HEIGHT + CLIENT_BUTTON_HEIGHT, DEMO_BUTTON_WIDTH, DEMO_BUTTON_HEIGHT, DemoButtonOnPress, MainMenu));
    SetButtonText(DemoButton, "UI Demo");
    SetButtonOutlineThickness(DemoButton, 2);
    SetButtonOutlineColor(DemoButton, LCD_BLUE);

    InfoButton = GetButton(AddButton((MAX_SCREEN_X >> 1), (TOP_PADDING << 1) + TITLE_SPACE + HOST_BUTTON_HEIGHT + CLIENT_BUTTON_HEIGHT, INFO_BUTTON_WIDTH, INFO_BUTTON_HEIGHT, InfoButtonOnPress, MainMenu));
    SetButtonText(InfoButton, "Info!");
    SetButtonOutlineThickness(InfoButton, 2);
    SetButtonOutlineColor(InfoButton, LCD_GRAY);

    SetVisible(MainMenu, true);
    GiveFocus(MainMenu);
}

void RenderMainMenu(void){
    if(Context->update == hardUpdate){
        Write((MAX_SCREEN_X >> 1) - (strlen(TITLE) << 3), TOP_PADDING, TITLE, 2, LCD_RED);
        Write((MAX_SCREEN_X >> 1) - (strlen(SUBTITLE) << 2), TOP_PADDING + 20, SUBTITLE, 1, LCD_WHITE);
    }
}

void HostButtonOnPress(void){
    while(InputPressed(TOUCH)){
        sleep(1);
    }
    HostButton->pressed = false;
    Flag(HostButton);                // flag for rendering

    sleep(15);

    SetVisible(MainMenu, false);
    InitRedstoneHost();

    FinishOnPress();
}

void ClientButtonOnPress(void){
    while(InputPressed(TOUCH)){
        sleep(1);
    }
    ClientButton->pressed = false;
    Flag(ClientButton);                // flag for rendering

    sleep(15);

    SetVisible(MainMenu, false);
    InitRedstoneClient();

    FinishOnPress();
}

void DemoButtonOnPress(void){
    while(InputPressed(TOUCH)){
        sleep(1);
    }
    DemoButton->pressed = false;
    Flag(DemoButton);                // flag for rendering

    sleep(15);

    SetVisible(MainMenu, false);
    DemoAppLaunch();

    FinishOnPress();
}

void InfoButtonOnPress(void){
    while(InputPressed(TOUCH)){
        sleep(1);
    }
    ClientButton->pressed = false;
    Flag(InfoButton);                // flag for rendering

    sleep(15);

    //SetVisible(MainMenu, false);

    FinishOnPress();
}

