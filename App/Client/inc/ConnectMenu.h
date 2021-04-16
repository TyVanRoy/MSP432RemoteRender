/*
 * ConnectMenu.h
 *
 *  Created on: Apr 24, 2020
 *      Author: tyvanroy
 */

#ifndef APP_CLIENT_INC_CONNECTMENU_H_
#define APP_CLIENT_INC_CONNECTMENU_H_

#include "RedUI.h"
#include "Redstone.h"

Window* ConnectMenu;

void ConnectButtonOnPress(void);
void ClientBackButtonOnPress(void);
void ClientInfoButtonOnPress(void);

void RenderConnectMenu(void);
void InitConnectMenu(void);

void ChangeStatus(ConnectionStatus newStatus);

#endif /* APP_CLIENT_INC_CONNECTMENU_H_ */
