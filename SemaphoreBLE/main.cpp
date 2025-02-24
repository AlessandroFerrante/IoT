/**
 * @file main.cpp
 * @author Alessandro Ferrante (github@alessandroferrante.net)
 * @brief 
 * @version 0.1
 * @date 2024-11-26
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "peripheral.h"
// #include "server.h" // disagree to upload the server, remember to comment on the import of the peripheral

BLEServer *serverBLE;
BLECharacteristic *caratteristicaLedVerde;
bool centralConnected = false;

bool connected = false;
bool deviceFound = false;
BLEScan *bleScan;
BLEAddress *pServerAddress;
BLERemoteCharacteristic *characteristic;

int stringToSemaphoreState(const String& value) {
    if (value == "RED") return RED;
    if (value == "YELLOW") return YELLOW;
    if (value == "GREEN") return GREEN;
    return YELLOW; // Default/fallback
}

void setup() {
    IoTBoard::init_serial();
    IoTBoard::init_leds();
    IoTBoard::init_display();
    Serial.println("Sistema inizializzato");
    display->println("Display enabled");
    display->display();

    #ifdef BLE_PERIFERICA
    Serial.println("Configurazione come periferica BLE");
    setupPeripheral();
    #endif

    #ifndef BLE_PERIFERICA
    Serial.println("Configurazione come dispositivo centrale BLE");
    setupServer();
    #endif
}

void loop() {
    // server
    #ifndef BLE_PERIFERICA
        loopServer();
    #endif

    // peripheal
    #ifdef BLE_PERIFERICA
        if (centralConnected) {
            Serial.println("Semaphore managed by the device");
        }
        else{
            digitalWrite(LED_YELLOW, HIGH);
            delay(1000);
            digitalWrite(LED_YELLOW, LOW);
            delay(1000);
        }
    
    #endif
}
