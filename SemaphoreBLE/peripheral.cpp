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

#include "peripheral.h"

// Declaration of global variables used by the peripheral
/*extern BLEServer *serverBLE;
extern BLECharacteristic *semaphoreCharacteristic;
extern bool centralConnected;
*/
enum {
    RED,
    YELLOW,
    GREEN
};

int stringToSemaphoreState(const String& value);
int colorSemaphore = YELLOW;

void updateSemaphore(int colorSemaphore){
    digitalWrite(LED_GREEN, LOW);
        digitalWrite(LED_YELLOW, LOW);
        digitalWrite(LED_RED, LOW);
        
        switch (colorSemaphore) {
            case GREEN:
                digitalWrite(LED_GREEN, HIGH);
                break;
            case YELLOW:
                digitalWrite(LED_YELLOW, HIGH);
                break;
            case RED:
                digitalWrite(LED_RED, HIGH);    
                break;
            default:
                break;
        }
}

// Callback class to manage writing events on the feature
class CallbacKSemaphore : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *caratteristicaScritta) override {
        // Sthe value received via BLE on the serial
        Serial.println(caratteristicaScritta->getValue());

        //Update the display with the value received
        display->clearDisplay();
        display->setCursor(0, 0);
        display->println("Valore Scritto");
        display->println(caratteristicaScritta->getValue());
        display->display();

        colorSemaphore = stringToSemaphoreState(caratteristicaScritta->getValue());
        updateSemaphore(colorSemaphore);
        
        // Notification to connected clients the new value of the feature
        caratteristicaScritta->notify();
    }
};

// Callback class to manage the events of the BLE server
class CallbacksServer : public BLEServerCallbacks {
    void onConnect(BLEServer *pServer) override {
        // Update the state of the connection
        centralConnected = true;

        // View on the display
        display->clearDisplay();
        display->setCursor(0, 0);
        display->println("Dispositivo Connesso");
        display->display();
    }

    void onDisconnect(BLEServer *pServer) override {
        // Update the state of the connection
        centralConnected = false;

        // View on the display
        display->clearDisplay();
        display->setCursor(0, 0);
        display->println("In attesa di connessione BLE");
        display->display();
    }
};

// Function to configure the device as a BLE peripheral
void setupPeripheral() {
    // Initialize the BLE device with a unique name
    BLEDevice::init("semaphore");

    // Create the BLE server and assign callbacks for events management
    serverBLE = BLEDevice::createServer();
    serverBLE->setCallbacks(new CallbacksServer());

    // Create a BLE service for light control
    BLEService *semaphoreService = serverBLE->createService(UUID_SERVICE_LIGHT);

    // Create a feature for green LED with reading, writing and notification properties
    semaphoreCharacteristic = semaphoreService->createCharacteristic(
                                        UUID_CHARATERISTIC_SEMAPHORE,
                                        BLECharacteristic::PROPERTY_WRITE | 
                                        BLECharacteristic::PROPERTY_READ | 
                                        BLECharacteristic::PROPERTY_NOTIFY
                                    );

    // Set initial value of the feature
    semaphoreCharacteristic->setValue("YELLOW");

    // Assign callback to manage writing events on the feature
    semaphoreCharacteristic->setCallbacks(new CallbacKSemaphore());

    // Start the BLE service
    semaphoreService->start();

    // Configures advertising to allow central devices to discover the peripheral
    BLEAdvertising *servizioAdvertising = BLEDevice::getAdvertising();
    servizioAdvertising->addServiceUUID(UUID_SERVICE_LIGHT); // Aggiunge l'UUID del servizio
    servizioAdvertising->setScanResponse(true);             // Permette risposte agli scan

    // Show a message on the display to indicate that the device is awaiting connection
    display->clearDisplay();
    display->setCursor(0, 0);
    display->println("In attesa di connessione BLE");
    display->display();

    // Start the Advertising BLE
    BLEDevice::startAdvertising();
}

#endif
