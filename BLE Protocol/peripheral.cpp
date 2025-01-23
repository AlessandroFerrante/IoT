/**
 * @file peripheral.cpp
 * @author Alessandro Ferrante (github@alessandroferrante.net)
 * @brief 
 * @version 0.1
 * @date 2024-12-17
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "peripheral.h"

// Declaration of global variables used by the peripheral
BLEServer *serverBLE;
BLECharacteristic *caratteristicaLedVerde;
bool centralConnected = false;

//Callback class to manage writing events on the feature
class CallbackLedVerde : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *caratteristicaScritta) override {
        // Print the value received via BLE on the serial
        Serial.println(caratteristicaScritta->getValue());

        // Update the display with the value received
        display->clearDisplay();
        display->setCursor(0, 0);
        display->println("Valore Scritto");
        display->println(caratteristicaScritta->getValue());
        display->display();

        // Check the value received and turn on/ off the green LED
        if (caratteristicaScritta->getValue() == "on") {
            digitalWrite(LED_GREEN, HIGH); // Lights the LED
        } else {
            digitalWrite(LED_GREEN, LOW); // turn off the LED
        }

        // Notification to connected clients the new value of the feature
        caratteristicaScritta->notify();
    }
};

// Callback class to manage the events of the BLE server
class CallbacksServer : public BLEServerCallbacks {
    void onConnect(BLEServer *pServer) override {
        //Updes the status of the connection and displays a message on the display
        centralConnected = true;
        display->clearDisplay();
        display->setCursor(0, 0);
        display->println("Dispositivo Connesso");
        display->display();
    }

    void onDisconnect(BLEServer *pServer) override {
        //Updes the status of the connection and displays a message on the display
        centralConnected = false;
        display->clearDisplay();
        display->setCursor(0, 0);
        display->println("In attesa di connessione BLE");
        display->display();
    }
};

// Function to configure the device as a BLE peripheral
void setupPeriferica() {
    // Initialize the BLE device with a unique name
    BLEDevice::init("lampada_philips");

    // Create the BLE server and assign callbacks for events management
    serverBLE = BLEDevice::createServer();
    serverBLE->setCallbacks(new CallbacksServer());

    //Create a BLE service for light control
    BLEService *servizioLuce = serverBLE->createService(UUID_SERVICE_LIGHT);

    // Create a feature for the green LED with reading, writing and notification properties
    caratteristicaLedVerde = servizioLuce->createCharacteristic(
        UUID_CHARATERISTIC_LED_GREEN,
        BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );

    // Sets the initial value of the feature
    caratteristicaLedVerde->setValue("off");

    // Assigns callback to manage writing events on the feature
    caratteristicaLedVerde->setCallbacks(new CallbackLedVerde());

    // Start the BLE service
    servizioLuce->start();

    // Configure advertising to allow central devices to discover the peripheral
    BLEAdvertising *servizioAdvertising = BLEDevice::getAdvertising();
    servizioAdvertising->addServiceUUID(UUID_SERVICE_LIGHT); //Adds the service of the service
    servizioAdvertising->setScanResponse(true);             // allows responses to scan

    // Show a message on the display to indicate that the device is awaiting connection
    display->clearDisplay();
    display->setCursor(0, 0);
    display->println("In attesa di connessione BLE");
    display->display();

    // Start the Advertising BLE
    BLEDevice::startAdvertising();
}

