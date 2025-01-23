/**
 * @file server.h
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
#include "ble_defines.h"
#include <iot_board.h>

// Global variables shared with the Main
extern BLEServer *serverBLE;
extern BLEScan *bleScan; // Object for the BLE scan
extern BLEAddress *pServerAddress; // Ble Server address
extern BLERemoteCharacteristic *characteristic; // Remote feature for interactions
extern bool connected;
extern bool deviceFound;
void setupServer();