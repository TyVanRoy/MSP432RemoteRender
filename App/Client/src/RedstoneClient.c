/*
 * RedstoneClient.c
 *
 *  Created on: Apr 24, 2020
 *      Author: tyvanroy
 */

#include "RedstoneClient.h"
#include "Redlib.h"
#include "RedUI.h"
#include "cc3100_usage.h"
#include "ConnectMenu.h"
#include "Canvas.h"

static ConnectionPacket_t ConnectionPacket;
static GameStatePacket_t GameStateBuffer;

static bool ControlFlag;

void InitRedstoneClient(void){
    G8RTOS_InitSemaphore(&CONTROLLOCK, 1);
    InitConnectMenu();
}

void LaunchRestoneClient(void){
}

/* Threads */

void InputCapture(void){

    ControlPacket_t oldControls;

    int16_t xc, yc;
    int16_t xa, za;

    Joystick_Init_Without_Interrupt();

    while(1){


        SemWait(&CONTROLLOCK);

        ControlPacket.inputField = Canvas->inputField;
        Canvas->inputField = GetInputField();
        GetJoystickCoordinates(&xc, &yc);

        ControlPacket.joystickX = xc > 0 ? ((int8_t)(((uint16_t) xc) >> 8 )) : -((int8_t)(((uint16_t) (-xc)) >> 8 ));
        ControlPacket.joystickY = yc > 0 ? ((int8_t)(((uint16_t) yc) >> 8 )) : -((int8_t)(((uint16_t) (-yc)) >> 8 ));
        ControlPacket.joystickX = (abs(ControlPacket.joystickX) < 3) ? 0 : ControlPacket.joystickX - 2;
        ControlPacket.joystickY = (abs(ControlPacket.joystickY) < 3) ? 0 : ControlPacket.joystickY - 2;

        // decayed average
        //ControlPacket.joystickX = (ControlPacket.joystickX + xc) >> 1;
        //ControlPacket.joystickY = (ControlPacket.joystickY + yc) >> 1;

        //while(bmi160_read_accel_x(&xa));
        //while(bmi160_read_accel_z(&za));

        //ControlPacket.accelX = xa > 0 ? ((int8_t)(((uint16_t) xa) >> 10 )) : -((int8_t)(((uint16_t) -xa) >> 10 ));
        //ControlPacket.accelZ = za > 0 ? ((int8_t)(((uint16_t) za) >> 10 )) : -((int8_t)(((uint16_t) -za) >> 10 ));

        ControlFlag = (
                (ControlPacket.accelX != oldControls.accelX) |
                (ControlPacket.accelZ != oldControls.accelZ) |
                (ControlPacket.joystickX != oldControls.joystickX) |
                (ControlPacket.joystickY != oldControls.joystickY) |
                (ControlPacket.inputField != oldControls.inputField) |
                (ControlPacket.hitAcknowledge != oldControls.hitAcknowledge) |
                (ControlPacket.missAcknowledge != oldControls.missAcknowledge) |
                (ControlPacket.ready != oldControls.ready)
        );

        oldControls.accelX = ControlPacket.accelX;
        oldControls.accelZ = ControlPacket.accelZ;
        oldControls.inputField = ControlPacket.inputField;
        oldControls.joystickX = ControlPacket.joystickX;
        oldControls.joystickY = ControlPacket.joystickY;
        oldControls.hitAcknowledge = ControlPacket.hitAcknowledge;
        oldControls.missAcknowledge = ControlPacket.missAcknowledge;
        oldControls.ready = ControlPacket.ready;

        SemSignal(&CONTROLLOCK);

        sleep(1);
        UnmaskField(Canvas->inputField);
        sleep(INPUT_CAP_PER - 1);
    }
}

void SendToHost(void){
    uint8_t* BUFFER_POS = ((uint8_t*) &ControlPacket);
    const uint16_t CLIENT_PACKET_SIZE = sizeof(ControlPacket);

    while(1){

        if(ControlFlag){
            SemWait(&WIFILOCK);
            SendData(BUFFER_POS, HOST_IP_ADDR, CLIENT_PACKET_SIZE);
            SemSignal(&WIFILOCK);

            ControlFlag = false;
        }

        sleep(SEND_HOST_PER);
    }

}

void ReceiveFromHost(void){
    uint8_t* BUFFER_POS = ((uint8_t*) &GameStateBuffer);
    const uint16_t HOST_PACKET_SIZE = sizeof(GameStateBuffer);

    while(1){
        SemWait(&WIFILOCK);
        while(ReceiveData(BUFFER_POS, HOST_PACKET_SIZE) < 0){
            SemSignal(&WIFILOCK);
            sleep(1);
            SemWait(&WIFILOCK);
        }
        SemSignal(&WIFILOCK);

        PushCanvasFrame(&GameStateBuffer);

        sleep(RECEIVE_HOST_PER);
    }

}


void ConnectToHost(void){

    // Connect to router
    initCC3100(Client);

    ChangeStatus(Connecting);

    ConnectionPacket.IP = getLocalIP();
    ConnectionPacket.playerNumber = 0;
    ConnectionPacket.acknowledge = true;
    ControlFlag = true;

    ControlPacket.ready = true;
    ControlPacket.playerID = 0;

    // Handshake
    SendData((uint8_t *) &ConnectionPacket, HOST_IP_ADDR, sizeof(ConnectionPacket));
    while(ReceiveData((uint8_t *) &GameStateBuffer, sizeof(GameStateBuffer)) != SUCCESS);

    ChangeStatus(Connected);

    sleep(500);

    SetVisible(ConnectMenu, false);
    InitCanvas();

    PushCanvasFrame(&GameStateBuffer);

    G8RTOS_AddThread(ReceiveFromHost, RECEIVE_HOST_PRI, "receive host");
    G8RTOS_AddThread(InputCapture, INPUT_CAP_PRI, "input capture");
    G8RTOS_AddThread(SendToHost, SEND_HOST_PRI, "send host");
    G8RTOS_KillSelf();
    while(1);
}
