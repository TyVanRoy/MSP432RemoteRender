/*
 * World.c
 *
 *  Created on: Apr 27, 2020
 *      Author: tyvanroy
 */
#include "World.h"
#include "RedstoneHost.h"
#include "Red3D.h"

#define LARGE_NUMBER 100000

static uint8_t WorldObjectCount = 0;

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

static bool RenderMode;

void InitWorld(void){
    G8RTOS_InitSemaphore(&WORLDLOCK, 1);

    RenderMode = true;

    /* Initialize the player */
    World.Players[0].camera.position.x = 0;
    World.Players[0].camera.position.y = 0;
    World.Players[0].camera.position.z = 25;
    World.Players[0].laser.charge = 0;
    World.Players[0].laser.status = la_asleep;

    /* For the hostless demo */
    Point3 position = {0, 0, -500};
    Point3 pVelocity = {0, 0, 0};
//    Point3 rVelocity = {0.004, 0.004, 0};
    Point3 rVelocity = {0.004, 0.004, 0};

    AddWorldObject(position, pVelocity, rVelocity, &Cube, 5, LCD_MAGENTA);

    position.x = 500;
    position.z = 0;
    position.y = 0;
    AddWorldObject(position, pVelocity, rVelocity, &Cube, 5, LCD_CYAN);
}

bool AddWorldObject(Point3 position, Point3 pVelocity, Point3 rVelocity, Poly* shape, float scale, uint16_t color){
    if(WorldObjectCount >= MAX_WORLD_OBJECTS){
        return false;
    }
    World.WorldObjects[WorldObjectCount].color = color;
    World.WorldObjects[WorldObjectCount].position.x = position.x;
    World.WorldObjects[WorldObjectCount].position.y = position.y;
    World.WorldObjects[WorldObjectCount].position.z = position.z;
    World.WorldObjects[WorldObjectCount].rotation.x = 0;
    World.WorldObjects[WorldObjectCount].rotation.y = 0;
    World.WorldObjects[WorldObjectCount].rotation.z = 0;
    World.WorldObjects[WorldObjectCount].pVelocity.x = pVelocity.x;
    World.WorldObjects[WorldObjectCount].pVelocity.y = pVelocity.y;
    World.WorldObjects[WorldObjectCount].pVelocity.z = pVelocity.z;
    World.WorldObjects[WorldObjectCount].rVelocity.x = rVelocity.x;
    World.WorldObjects[WorldObjectCount].rVelocity.y = rVelocity.y;
    World.WorldObjects[WorldObjectCount].rVelocity.z = rVelocity.z;
    ClonePoly(&World.WorldObjects[WorldObjectCount].shape, shape, scale);
    World.WorldObjects[WorldObjectCount].scale = scale;
    WorldObjectCount++;

    return true;
}

void InsertWorldObject(WorldObject_t insert, uint8_t index){
    World.WorldObjects[index].color = insert.color;
    World.WorldObjects[index].position.x = insert.position.x;
    World.WorldObjects[index].position.y = insert.position.y;
    World.WorldObjects[index].position.z = insert.position.z;
    World.WorldObjects[index].rotation.x = insert.rotation.x;
    World.WorldObjects[index].rotation.y = insert.rotation.y;
    World.WorldObjects[index].rotation.z = insert.rotation.z;
    World.WorldObjects[index].pVelocity.x = insert.pVelocity.x;
    World.WorldObjects[index].pVelocity.y = insert.pVelocity.y;
    World.WorldObjects[index].pVelocity.z = insert.pVelocity.z;
    World.WorldObjects[index].rVelocity.x = insert.rVelocity.x;
    World.WorldObjects[index].rVelocity.y = insert.rVelocity.y;
    World.WorldObjects[index].rVelocity.z = insert.rVelocity.z;
    ClonePoly(&World.WorldObjects[index].shape, &insert.shape, insert.scale);
    World.WorldObjects[index].scale = insert.scale;
}

void RemoveWorldObject(uint8_t index){
    if(index >= WorldObjectCount){
        return;
    }
    if(index < WorldObjectCount - 1){
        InsertWorldObject(World.WorldObjects[WorldObjectCount - 1], index);
    }
    WorldObjectCount--;
}

void RemoveAllWorldObjects(void){
    WorldObjectCount = 0;
}

void ToggleRenderMode(void){
    RenderMode = !RenderMode;
}

void ResetPlayer(void){
    World.Players[0].camera.position.x = 0;
    World.Players[0].camera.position.y = 0;
    World.Players[0].camera.position.z = -25;
    World.Players[0].camera.rotation.x = 0;
    World.Players[0].camera.rotation.y = 0;
    World.Players[0].camera.rotation.z = 0;
    World.Players[0].laser.charge = 0;
    World.Players[0].laser.status = la_asleep;
}

inline void ComputeFrame(Frame_t* frame, Point3* projection){

    frame->lineCount = 0;
    Point3 origin = {0, 0, 0};
    int objectsCounted = 0; // for debugging
    for(int i = 0; i < WorldObjectCount; i++){

        // Create a projection, use projection[0] as origin
        projection[0].x = World.WorldObjects[i].position.x;
        projection[0].y = World.WorldObjects[i].position.y;
        projection[0].z = World.WorldObjects[i].position.z;

        // Rotate projection around the camera
        TranslatePoint(&projection[0], VectorScale(World.Players[0].camera.position, -1));
        RotatePoint(&projection[0], origin, VectorScale(World.Players[0].camera.rotation, -1));

        /* Check if the object is in visible range */
        if(projection[0].z > 0){
            continue;
        }else{
            Point p = WorldToScreenPoint(origin, projection[0]);
            if((p.x < 0 || p.x >= MAX_SCREEN_X) || (p.y < 0 || p.y >= MAX_SCREEN_Y)){
                continue;
            }
        }

        // Create projection
        CloneToPosition(&projection[1], World.WorldObjects[i].shape.points, projection[0], World.WorldObjects[i].shape.pointCount);

        // Rotate projection around origin
        RotatePoints(&projection[1], World.WorldObjects[i].shape.pointCount, projection[0], World.WorldObjects[i].rotation);

        PrintPoly(World.Players[0].camera.position, &projection[1], projection[0], World.WorldObjects[i].shape.lines, World.WorldObjects[i].shape.lineCount, World.WorldObjects[i].color, frame, frame->lineCount, RenderMode);

        objectsCounted++;
    }
}

static inline bool AimIntersect(WorldObject_t object, Point3* hit){

    Point3 pDir = {World.Players[0].camera.position.x, World.Players[0].camera.position.y, World.Players[0].camera.position.z + 1};
    RotatePoint(&pDir, World.Players[0].camera.position, World.Players[0].camera.rotation);
    pDir = VectorSub(World.Players[0].camera.position, pDir);
    Point3 oDir = VectorSub(World.Players[0].camera.position, object.position);

    if(DotProduct(pDir, oDir) == 0){
        return false;
    }

    Point3 intersection = Intersection(pDir, World.Players[0].camera.position, oDir, object.position);

    if(DistanceBetween(intersection, object.position) <= object.scale){
        hit->x = intersection.x;
        hit->y = intersection.y;
        hit->z = intersection.z;
        return true;
    }

    return false;
}

static inline int FireLaser(void){

    int object = -1;
    float min = LARGE_NUMBER;
    float d;
    Point3 hit = {0, 0, 0};

    for(int i = 0; i < WorldObjectCount; i++){
        if(AimIntersect(World.WorldObjects[i], &hit)) {
            d = DistanceBetween(World.Players[0].camera.position, hit);
            if (d < min) {
                object = i;
                min = d;
            }
        }
    }

    return object;
}

static inline void BlowUp(uint8_t index){

    Point3 r = {0, HALF_PI, 0};
    Point3 v1 = VectorSub(World.Players[0].camera.position, World.WorldObjects[index].position);
    RotatePoint(&v1, World.WorldObjects[index].position, r);
    v1 = VectorScale(Normalized(v1), 3);
    Point3 v2 = VectorScale(v1, -1);

    AddWorldObject(World.WorldObjects[index].position, v1, World.WorldObjects[index].rVelocity, &Cube, (World.WorldObjects[index].scale / 2), World.WorldObjects[index].color);
    AddWorldObject(World.WorldObjects[index].position, v2, World.WorldObjects[index].rVelocity, &Cube, (World.WorldObjects[index].scale / 2), World.WorldObjects[index].color);

    RemoveWorldObject(index);
}

/* This is the GameEngine tick */
void Tick(void){

    Point3 origin = {0, 0, 0};
    Point3 forward = {0, 0, -1};
    Point3 rotation = {0, 0, 0};
    Point3 direction;

    float magnitude;

    bool reversed = false;
    bool centered = false;
    bool fire = false;

    while(1){

        SemWait(&WORLDLOCK);

        /*
         *  Update the player's laser
         */
        LaserStatus status = World.Players[0].laser.status;
        if(status == la_hit){
            if(World.Players[0].controls.hitAcknowledge){
                World.Players[0].laser.status = la_asleep;
            }
        }else if(status == la_miss){
            if(World.Players[0].controls.missAcknowledge){
                World.Players[0].laser.status = la_asleep;
            }
        }else if((World.Players[0].controls.inputField & LASER_BUTTON) && (fire == false)){
            int object = FireLaser();
            if(object > -1){
                BlowUp(object);
                World.Players[0].laser.status = la_hit;
            }else{
                World.Players[0].laser.status = la_miss;
            }
            fire = true;
        }else{
            fire = false;
        }

        magnitude = Magnitude(World.Players[0].velocity);

        /*
         *  Apply thrust if the player is thrusting && magnitude of velocity < Thrust Capacity
         */
        if((World.Players[0].controls.inputField & THRUST_BUTTON) && (magnitude < THRUST_CAPACITY)){

            // Find the direction player is facing (direction is automatically normalized
            direction = VectorAdd(forward, origin);
            RotatePoint(&direction, origin, World.Players[0].camera.rotation);

            // Velocity += thrust - (friction if using friction)
            World.Players[0].velocity = VectorAdd(World.Players[0].velocity, VectorScale(direction, THRUST_CONTROL));

            // recalculate magnitude for the breaks
            magnitude = Magnitude(World.Players[0].velocity);
        }

        /*
         *  Apply breaks if the player is breaking && magnitude of velocity > 0
         */
        if((World.Players[0].controls.inputField & BREAK_BUTTON) && (magnitude > 0)){
            if(magnitude < BREAK_CUTOFF){
                World.Players[0].velocity.x = 0;
                World.Players[0].velocity.y = 0;
                World.Players[0].velocity.z = 0;
            }else{
                // Apply breaks opposite to direction of player velocity
                World.Players[0].velocity = VectorAdd(World.Players[0].velocity, VectorScale(Normalized(World.Players[0].velocity), -BREAK_CONTROL));
            }
        }

        /*
         *  Reverse the player if they request it
         */
        if(World.Players[0].controls.inputField & REVERSE_BUTTON){
            if(!reversed){
                rotation.y += PI;
                reversed = true;
            }
        }else if(reversed){
            reversed = false;
        }

        /*
         *  Center the player's camera rotation if they request it
         */
        if(World.Players[0].controls.inputField & CENTER_BUTTON){
            if(!centered){
                World.Players[0].camera.rotation.x = 0;
                World.Players[0].camera.rotation.y = 0;
                World.Players[0].camera.rotation.z = 0;
                centered = true;
            }
        }else if(centered){
            centered = false;
        }


        /*
         *  Move and rotate Player
         */
        rotation.x = -(World.Players[0].controls.joystickY) * ROTATION_CONTROL;
        rotation.y = (World.Players[0].controls.joystickX) * ROTATION_CONTROL;

        TranslatePoint(&World.Players[0].camera.position, World.Players[0].velocity);
        TranslatePoint(&World.Players[0].camera.rotation, rotation);

        /*
         *  Move & rotate WorldObjects
         */
        for(int i = 0; i < WorldObjectCount; i++){
            TranslatePoint(&World.WorldObjects[i].position, World.WorldObjects[i].pVelocity);
            TranslatePoint(&World.WorldObjects[i].rotation, World.WorldObjects[i].rVelocity);
        }
        SemSignal(&WORLDLOCK);

        sleep(WORLD_TICK_PER);
    }
}
