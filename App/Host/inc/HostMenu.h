/*
 * HostMenu.h
 *
 *  Created on: Apr 28, 2020
 *      Author: tyvanroy
 */

#ifndef APP_HOST_INC_HOSTMENU_H_
#define APP_HOST_INC_HOSTMENU_H_

#include "RedUI.h"
#include "Redstone.h"

Window* HostMenu;

int SpawnDepth;
int SpawnScale;
int SpawnxVel;
int SpawnyVel;
int SpawnzVel;
int SpawnxRot;
int SpawnyRot;
int SpawnzRot;

void RenderHostMenu(void);
void InitHostMenu(void);
void LaunchSeverButtonOnPress(void);
void DeleteObjectsButtonOnPress(void);
void ResetPlayerButtonOnPress(void);
void SpawnObjectButtonOnPress(void);
void ToggleRenderButtonOnPress(void);
void SetToWaiting(void);
void SetToDeployed(void);


#endif /* APP_HOST_INC_HOSTMENU_H_ */
