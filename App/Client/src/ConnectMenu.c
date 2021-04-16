/*
 * ConnectMenu.c
 *
 *  Created on: Apr 24, 2020
 *      Author: tyvanroy
 */

#include "ConnectMenu.h"
#include "MainMenu.h"
#include "RedLib.h"
#include "RedstoneClient.h"
#include <string.h>

#define TITLE                   "3D Portal"
#define SUBTITLE                "Connect to a rendering stream!"
#define TOP_PADDING             7

#define TITLE_SPACE             50

#define CONNECT_BUTTON_WIDTH    (MAX_SCREEN_X - 40)
#define CONNECT_BUTTON_HEIGHT   70

#define BACK_BUTTON_WIDTH       ((MAX_SCREEN_X - 40) >> 1)
#define BACK_BUTTON_HEIGHT      60
#define INFO_BUTTON_WIDTH      ((MAX_SCREEN_X - 40) >> 1)
#define INFO_BUTTON_HEIGHT       60

static ConnectionStatus status = NotConnected;

static Button* ConnectButton;
static Button* BackButton;
static Button* InfoButton;

void ChangeStatus(ConnectionStatus newStatus){
    status = newStatus;
    switch(status){
        case NotConnected:
            SetButtonText(ConnectButton, "Not connected...");
            SetButtonBackgroundColor(ConnectButton, LCD_RED);
            break;
        case Connecting:
            SetButtonText(ConnectButton, "Connecting...");
            SetButtonBackgroundColor(ConnectButton, LCD_YELLOW);
            break;
        case Connected:
            SetButtonText(ConnectButton, "Connected!");
            SetButtonBackgroundColor(ConnectButton, LCD_GREEN);
            break;
    }
}

void RenderConnectMenu(void){
    if(Context->update == hardUpdate){
        Write((MAX_SCREEN_X >> 1) - (strlen(TITLE) << 3), TOP_PADDING, TITLE, 2, LCD_RED);
        Write((MAX_SCREEN_X >> 1) - (strlen(SUBTITLE) << 2), TOP_PADDING + 20, SUBTITLE, 1, LCD_WHITE);
    }
}

void ConnectButtonOnPress(void){
    while(InputPressed(TOUCH)){
        sleep(1);
    }
    ConnectButton->pressed = false;
    Flag(ConnectButton);                // flag for rendering

    sleep(15);

    if(status == NotConnected){
        G8RTOS_AddThread(ConnectToHost, 0xAA, "connect to host");
        ChangeStatus(NotConnected);
    }

    FinishOnPress();
}

void ClientBackButtonOnPress(void){
    while(InputPressed(TOUCH)){
        sleep(1);
    }
    BackButton->pressed = false;
    Flag(BackButton);                // flag for rendering

    sleep(15);

    SetVisible(ConnectMenu, false);
    SetVisible(MainMenu, true);

    FinishOnPress();
}

void ClientInfoButtonOnPress(void){
    while(InputPressed(TOUCH)){
        sleep(1);
    }
    InfoButton->pressed = false;
    Flag(InfoButton);                // flag for rendering

    sleep(15);

    FinishOnPress();
}

void InitConnectMenu(void){
    ConnectMenu = GetWindow(AddWindow(0, 0, MAX_SCREEN_X, MAX_SCREEN_Y, TOP_LAYER, RenderConnectMenu));

    ConnectButton = GetButton(AddButton((MAX_SCREEN_X >> 1) - (CONNECT_BUTTON_WIDTH >> 1), (TOP_PADDING << 1) + 50, CONNECT_BUTTON_WIDTH, CONNECT_BUTTON_HEIGHT, ConnectButtonOnPress, ConnectMenu));
    SetButtonText(ConnectButton, "Connect to Host");
    SetButtonOutlineThickness(ConnectButton, 3);
    SetButtonOutlineColor(ConnectButton, LCD_WHITE);
    SetButtonBackgroundColor(ConnectButton, LCD_BLUE);

    BackButton = GetButton(AddButton((MAX_SCREEN_X >> 1) - (BACK_BUTTON_WIDTH), (TOP_PADDING << 1) + TITLE_SPACE + CONNECT_BUTTON_HEIGHT, BACK_BUTTON_WIDTH, BACK_BUTTON_HEIGHT, ClientBackButtonOnPress, ConnectMenu));
    SetButtonText(BackButton, "<- Back   ");
    SetButtonOutlineThickness(BackButton, 2);
    SetButtonOutlineColor(BackButton, LCD_GRAY);

    InfoButton = GetButton(AddButton((MAX_SCREEN_X >> 1), (TOP_PADDING << 1) + TITLE_SPACE + CONNECT_BUTTON_HEIGHT, INFO_BUTTON_WIDTH, INFO_BUTTON_HEIGHT, ClientInfoButtonOnPress, ConnectMenu));
    SetButtonText(InfoButton, "Info!");
    SetButtonOutlineThickness(InfoButton, 2);
    SetButtonOutlineColor(InfoButton, LCD_BLUE);

    SetVisible(ConnectMenu, true);
    GiveFocus(ConnectMenu);
}
