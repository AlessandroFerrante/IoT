/**
 * @file BLEbtn.h
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
#include "iot_board.h"
extern BLEServer *serverBLE;
extern BLEScan *bleScan;
extern BLEAddress *pServerAddress; 
extern BLERemoteCharacteristic *remoteFeature; 
extern bool toogleLamp;