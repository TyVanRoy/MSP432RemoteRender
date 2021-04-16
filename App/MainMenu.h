/*
 * MainMenu.h
 *
 *  Created on: Apr 28, 2020
 *      Author: tyvanroy
 */

#ifndef APP_MAINMENU_H_
#define APP_MAINMENU_H_

#include "RedUI.h"
#include "Redstone.h"

Window* MainMenu;

void InitMainMenu(void);
void RenderMainMenu(void);

void HostButtonOnPress(void);
void ClientButtonOnPress(void);
void DemoButtonOnPress(void);
void InfoButtonOnPress(void);

#endif /* APP_MAINMENU_H_ */
