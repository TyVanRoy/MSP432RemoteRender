/*
 * SpawnPlane.h
 *
 *  Created on: Apr 30, 2020
 *      Author: tyvanroy
 */

#ifndef APP_HOST_INC_SPAWNPLANE_H_
#define APP_HOST_INC_SPAWNPLANE_H_

#include "RedLib.h"

Window* SpawnPlane;

void InitSpawnPlane(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
void RenderSpawnPlane(void);
void OnTouchSpawnPlane(void);

#endif /* APP_HOST_INC_SPAWNPLANE_H_ */
