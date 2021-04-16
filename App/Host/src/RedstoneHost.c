/*
 * RedstoneHost.c
 *
 *  Created on: Apr 24, 2020
 *      Author: tyvanroy
 */

#include "RedstoneHost.h"
#include "Redstone.h"
#include "HostMenu.h"
#include "World.h"
#include "demo_sysctl.h"
#include "cc3100_usage.h"

/* Game State */
static ConnectionPacket_t ConnectionPacketBuffer;
static ControlPacket_t ControlPacketBuffer;
static GameStatePacket_t GameStatePacket;

static Frame_t Frame;

void InitRedstoneHost(void){
    InitHostMenu();
    InitWorld();
}

void LaunchRedstoneHost(void){
    G8RTOS_AddThread(WaitForClient, 0, "wait for client");
}

static inline void FillGameStatePacket(Frame_t frame){

    GameStatePacket.frame.lineCount = frame.lineCount;
    for(int i = 0; i < frame.lineCount; i++){
        GameStatePacket.frame.lines[i].color = frame.lines[i].color;
        GameStatePacket.frame.lines[i].a.x = frame.lines[i].a.x;
        GameStatePacket.frame.lines[i].a.y = frame.lines[i].a.y;
        GameStatePacket.frame.lines[i].b.x = frame.lines[i].b.x;
        GameStatePacket.frame.lines[i].b.y = frame.lines[i].b.y;
    }

    GameStatePacket.laser.charge = World.Players[0].laser.charge;
    GameStatePacket.laser.status = World.Players[0].laser.status;
    GameStatePacket.x = (int) (World.Players[0].camera.position.x);
    GameStatePacket.y = (int) (World.Players[0].camera.position.y);
    GameStatePacket.z = ((int) (World.Players[0].camera.position.z)) - 25;
}

static inline void EmptyControlPacket(void){
    uint8_t id = ControlPacketBuffer.playerID;
    World.Players[id].controls.accelX = ControlPacketBuffer.accelX;
    World.Players[id].controls.accelZ = ControlPacketBuffer.accelZ;
    World.Players[id].controls.inputField = ControlPacketBuffer.inputField;
    World.Players[id].controls.joystickX = ControlPacketBuffer.joystickX;
    World.Players[id].controls.joystickY = ControlPacketBuffer.joystickY;
    World.Players[id].controls.hitAcknowledge = ControlPacketBuffer.hitAcknowledge;
    World.Players[id].controls.missAcknowledge = ControlPacketBuffer.missAcknowledge;
    World.Players[id].controls.ready = ControlPacketBuffer.ready;
}

/*
 * Threads
 */

/* Compute & send each player's frame */
void SendToClient(void){

    uint8_t* BUFFER_POS = ((uint8_t*) &GameStatePacket);
    const uint16_t HOST_PACKET_SIZE = sizeof(GameStatePacket);

    Point3 projection[16];

    while(1){

        ComputeFrame(&Frame, projection);
        FillGameStatePacket(Frame);

        SemWait(&WIFILOCK);
        SendData(BUFFER_POS, World.Players[0].IP, HOST_PACKET_SIZE);
        SemSignal(&WIFILOCK);

        sleep(SEND_CLIENT_PER);
    }
}

/* Receive control input from Players */
void ReceiveFromClient(void){
    uint8_t* BUFFER_POS = ((uint8_t*) &ControlPacketBuffer);
    const uint16_t CLIENT_PACKET_SIZE = sizeof(ControlPacketBuffer);

    while(1){
        SemWait(&WIFILOCK);
        while(ReceiveData(BUFFER_POS, CLIENT_PACKET_SIZE) < 0){
            SemSignal(&WIFILOCK);
            sleep(1);
            SemWait(&WIFILOCK);
        }
        SemSignal(&WIFILOCK);

        SemWait(&WORLDLOCK);
        EmptyControlPacket();
        SemSignal(&WORLDLOCK);

        sleep(RECEIVE_CLIENT_PER);
    }
}

void WaitForClient(void){

    SetToWaiting();

    sleep(50);

    initCC3100(Host);

    // wait for client
    while(ReceiveData((uint8_t *) &ConnectionPacketBuffer, sizeof(ConnectionPacketBuffer)) != SUCCESS);

    World.Players[0].IP = ConnectionPacketBuffer.IP;

    GameStatePacket.frame.lineCount = 0;

    sleep(5);

    SendData((uint8_t *) &GameStatePacket, World.Players[0].IP, sizeof(GameStatePacket));

    SetToDeployed();

    sleep(50);

    G8RTOS_AddThread(Tick, TICK_PRI, "world tick");
    G8RTOS_AddThread(SendToClient, SEND_CLIENT_PRI, "send client");
    G8RTOS_AddThread(ReceiveFromClient, RECEIVE_CLIENT_PRI, "receive client");
    G8RTOS_KillSelf();

    while(1);
}

