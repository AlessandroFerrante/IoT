/**
 * @file server.h
 * @author Alessandro Ferrante (github@alessandroferrante.net)
 * @brief 
 * @version 0.1
 * @date 2024-11-26
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef SERVER_H
#define SERVER_H

#include <BLEDevice.h>
#include <BLEAdvertisedDevice.h>
#include "ble_defines.h"
#include <iot_board.h>
#include <iostream>
#include <list>

#define MAX_SLAVES 3
int countSlave = 0;

// Global variables shared with the main
extern bool connected; // Ble connection status
extern bool deviceFound; // Flag indicating if the device has been found
extern BLEScan *bleScan; // object for the BLE scan
extern BLEAddress *pServerAddress; // Ble Server address
extern BLERemoteCharacteristic *characteristic; // Remote feature for interactions
void setupServer();
void loopServer();