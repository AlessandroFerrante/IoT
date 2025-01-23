/**
 * @file peripheral.h
 * @author Alessandro Ferrante (github@alessandroferrante.net)
 * @brief 
 * @version 0.1
 * @date 2024-12-17
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "ble_defines.h"
#include "iot_board.h"

void connectToLampada();
void setupPeriferica();

extern BLEServer *serverBLE;
extern BLECharacteristic *caratteristicaLedVerde;
extern bool centralConnected;
