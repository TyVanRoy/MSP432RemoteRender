/*
 * Redstone.h
 *
 *  Created on: Apr 24, 2020
 *      Author: tyvanroy
 */

#ifndef APP_INC_REDSTONE_H_
#define APP_INC_REDSTONE_H_

#include "RedstoneHost.h"
#include "RedstoneClient.h"
#include "RedLib.h"
#include "G8RTOS_Semaphores.h"
#include <stdbool.h>

/* Thread Priorities */
#define RECEIVE_HOST_PRI    0xAA
#define SEND_HOST_PRI       0xAA
#define INPUT_CAP_PRI       0xAA

#define RECEIVE_CLIENT_PRI  0xAB
#define SEND_CLIENT_PRI     0xAB
#define TICK_PRI            0xAB


/* Thread Periods */
#define SEND_CLIENT_PER     30
#define RECEIVE_HOST_PER    10
#define SEND_HOST_PER       10
#define RECEIVE_CLIENT_PER  3
#define WORLD_TICK_PER      30
#define INPUT_CAP_PER       15

/* World Settings */
#define WAIT_TIME           1000
#define MAX_PLAYERS         1
#define MAX_WORLD_OBJECTS   10

/* Movement Settings */
#define VIEW_THRESHOLD      0
#define ROTATION_CONTROL    0.0005
#define THRUST_CONTROL      0.05
#define THRUST_CAPACITY     3
#define BREAK_CONTROL       (THRUST_CONTROL * 3)
#define BREAK_CUTOFF        0.1

/* Laser Settings */
#define CHARGE_TIME     1000
#define FULL_CHARGE     (CHARGE_TIME / INPUT_CAP_PER)

/* Controls */
#define THRUST_BUTTON   B1
#define BREAK_BUTTON    B3
#define LASER_BUTTON    B0
#define REVERSE_BUTTON  B4
#define CENTER_BUTTON   B2


/*
 *  Types
 */

typedef enum Page
{
    MainMenuPage,
    ConnectMenuPage,
    HostMenuPage,
    DemoPage,
    InfoPage
} Page;

typedef struct WorldObject_t{
    Poly shape;
    float scale;
    Point3 position;
    Point3 rotation;
    Point3 pVelocity;
    Point3 rVelocity;
    uint16_t color;
} WorldObject_t;

typedef enum LaserStatus{
    la_hit,
    la_miss,
    la_charging,
    la_charged,
    la_asleep
} LaserStatus;

typedef struct LaserInfo{
    uint8_t charge;
    LaserStatus status;
} LaserInfo;

/* Client -> Host (Controller input data) */
typedef struct ControlPacket_t{
    uint8_t playerID;
    int8_t accelX;
    int8_t accelZ;
    int8_t joystickX;
    int8_t joystickY;
    uint8_t inputField;
    bool hitAcknowledge;
    bool missAcknowledge;
    bool ready;
} ControlPacket_t;

typedef struct Player{
    uint32_t IP;
    Camera camera;
    Point3 velocity;
    ControlPacket_t controls;
    LaserInfo laser;
} Player;

/* Data used to calculate the GameStatePacket */
typedef struct WorldState_t{
    Player Players[MAX_PLAYERS];
    WorldObject_t WorldObjects[MAX_WORLD_OBJECTS];
} WorldState_t;

typedef enum ConnectionStatus{
    NotConnected,
    Connecting,
    Connected
} ConnectionStatus;

/* Used to initialize connection with Host */
typedef struct ConnectionPacket_t{
    uint32_t IP;
    uint8_t playerNumber;
    bool acknowledge;
} ConnectionPacket_t;

/* Host -> Client (Frame/game data)*/
typedef struct GameStatePacket_t{
    Frame_t frame;
    LaserInfo laser;
    /* These are just to be rendered as text to show the client where they are */
    int32_t x;
    int32_t y;
    int32_t z;
} GameStatePacket_t;


static bool Hosting;
semaphore_t WIFILOCK;

void InitRedstone(void);
void LaunchRedstone(void);
bool DetermineHost(void);

#endif /* APP_INC_REDSTONE_H_ */
