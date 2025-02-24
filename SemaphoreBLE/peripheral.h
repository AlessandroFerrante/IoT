/**
 * @file peripheral.h
 * @author Alessandro Ferrante (github@alessandroferrante.net)
 * @brief 
 * @version 0.1
 * @date 2024-11-26
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef PERIFERICA_H
#define PERIFERICA_H

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "ble_defines.h"
#include <iot_board.h>
#include <string.h>
using namespace std;

// Declaration of global variables used by the peripheral
extern BLEServer *serverBLE;
extern BLECharacteristic *semaphoreCharacteristic;
extern bool centralConnected;
void setupPeripheral();
