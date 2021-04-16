/*
 * SpawnPlane.c
 *
 *  Created on: Apr 30, 2020
 *      Author: tyvanroy
 */

#include "SpawnPlane.h"
#include "HostMenu.h"
#include "World.h"
#include <string.h>

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

void InitSpawnPlane(uint16_t x, uint16_t y, uint16_t width, uint16_t height){

    SpawnPlane = GetWindow(AddWindow(x, y, width, height, TOP_LAYER, RenderSpawnPlane));
    SetOnTouchHandler(SpawnPlane, OnTouchSpawnPlane);

    SetVisible(SpawnPlane, true);
}

void RenderSpawnPlane(void){
    if(Context->update == hardUpdate){
        DrawRect(0, 0, Context->width, Context->height, 1, LCD_WHITE);
        Write(0, 0, "XY Spawn Plane", 1, LCD_WHITE);
    }
}

void OnTouchSpawnPlane(void){


    while(InputPressed(TOUCH));

    int x = (FocusWindow->touchPoint.x - SpawnPlane->x) - (SpawnPlane->width >> 1);
    int y = (FocusWindow->touchPoint.y - SpawnPlane->y) - (SpawnPlane->height >> 1);

    Point3 position = {-x, y, (-(SpawnDepth * 10)) + 25};
    Point3 pVelocity = {SpawnxVel, SpawnyVel, SpawnzVel};
    float pp = (PI * .0005555556);
    Point3 rVelocity = {pp * ((float) SpawnxRot), pp * ((float) SpawnyRot), pp * ((float) SpawnzRot)};
    AddWorldObject(position, pVelocity, rVelocity, &Cube, SpawnScale, LCD_MAGENTA);

    FinishOnTouch();
}
