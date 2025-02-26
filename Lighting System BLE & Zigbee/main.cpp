/**
 * @file main.cpp
 * @author Alessandro Ferrante (github@alessandroferrante.net)
 * @brief 
 * @version 0.1
 * @date 2024-12-17
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include <BLEDevice.h>
#include <BLEAdvertisedDevice.h>
#include "iot_board.h"

//for different devices
#define GATEWAY 
//#define BLE_BTN 
//#define ZGB_LAMP 

#ifdef GATEWAY
#include "gateway.h"
#endif

#ifdef BLE_BTN
#include "BLEbtn.h"
#endif

#ifdef ZGB_LAMP
#include "lamp.h"
#endif

bool lampPower = false;
bool toogleLamp;
void onBtn1Release();
void onBtn1Pressed();

void onBtn1Release() {
    detachInterrupt(BTN_1);
    attachInterrupt(BTN_1, onBtn1Pressed, FALLING);
    toogleLamp = true;
}

void onBtn1Pressed() {
    detachInterrupt(BTN_1);
    attachInterrupt(BTN_1, onBtn1Release, RISING);
}

// Function to alternate the status of the green LED
void switchLamp() {
    // server
    #ifdef BLE_BTN
    lampPower = !lampPower;
    if (lampPower) 
        remoteFeature->writeValue("on");
    else 
        remoteFeature->writeValue("off");
    #endif

    // gateway
    #ifdef GATEWAY
    if (featureLamp->getValue() == "on") 
        featureLamp->setValue("off");
    else 
        featureLamp->setValue("on");

    featureLamp->notify();
    #endif

    #ifdef ZGB_LAMP

    #endif
}

void setup() {
    IoTBoard::init_serial();
    IoTBoard::init_leds();
    IoTBoard::init_display();
    display->println("Display enabled");
    display->display();
    
    #ifndef ZGB_LAMP
    pinMode(BTN_1, INPUT_PULLUP);
    attachInterrupt(BTN_1, onBtn1Pressed, FALLING);
    #endif

    #ifdef GATEWAY
    setupGateway();
    #endif

    #ifdef BLE_BTN
    setupBLEbtn();
    #endif
    
    #ifdef ZGB_LAMP
    setupLamp();
    #endif
}

// ? implemented in device.h
void loop() {
    // BLEserver
    #ifdef BLE_BTN
        loopBLEbtn();
    #endif

    // BLEgateway - coordinatorZigBEE
    #ifdef GATEWAY
        loopGateway();
    #endif

    // zigbee lamp
    #ifdef ZGB_LAMP
        loopLamp();
    #endif
}