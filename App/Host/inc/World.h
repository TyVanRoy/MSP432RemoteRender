/*
 * World.h
 *
 *  Created on: Apr 27, 2020
 *      Author: tyvanroy
 */

#ifndef APP_HOST_INC_WORLD_H_
#define APP_HOST_INC_WORLD_H_

#include "G8RTOS_Semaphores.h"
#include "Redstone.h"
#include "RedLib.h"

WorldState_t World;
semaphore_t WORLDLOCK;

void InitWorld(void);
void Tick(void);
inline void ComputeFrame(Frame_t* frame, Point3* projection);
bool AddWorldObject(Point3 position, Point3 pVelocity, Point3 rVelocity, Poly* shape, float scale, uint16_t color);
void InsertWorldObject(WorldObject_t insert, uint8_t index);
void RemoveWorldObject(uint8_t index);
void RemoveAllWorldObjects(void);
void ToggleRenderMode(void);
void ResetPlayer(void);


#endif /* APP_HOST_INC_WORLD_H_ */
