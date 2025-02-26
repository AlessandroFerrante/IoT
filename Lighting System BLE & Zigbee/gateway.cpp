/**
 * @file gateway.h
 * @author Alessandro Ferrante (github@alessandroferrante.net)
 * @brief 
 * @version 0.1
 * @date 2024-12-18
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "gateway.h"


// variables shared with main

BLEServer *serverBLE;
BLECharacteristic *featureLamp;
bool centralConnected;
bool power = false;

extern void switchLamp();

// callback class to handle write events on the feature
class CallbackLamp : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *feature) override {
    
        Serial.println(feature->getValue());

        display->clearDisplay();
        display->setCursor(0, 0);
        display->println("Value ");
        display->println(feature->getValue());
        display->display();

        if (feature->getValue() == "on") 
            power = true;
        else if (feature->getValue() == "off") 
            power = false;

        zigbee->startPacket();
        zigbee->writeBytes((uint8_t *)&power, sizeof(power));
        zigbee-> sendPacket(0x02, true);

        // notifies connected clients of the new characteristic value
        feature->notify();
    }
};

// Callback class to handle server events
class CallbacksServer : public BLEServerCallbacks {
    void onConnect(BLEServer *pServer) override {
        centralConnected = true;
        display->clearDisplay();
        display->setCursor(0, 0);
        display->println("Device Connected");
        display->display();
    }

    void onDisconnect(BLEServer *pServer) override {
        centralConnected = false;
        display->clearDisplay();
        display->setCursor(0, 0);
        display->println("Waiting for BLE connection");
        display->display();
    }
};

void setupGateway() {

    BLEDevice::init("gateway_lamp");

    serverBLE = BLEDevice::createServer();
    serverBLE->setCallbacks(new CallbacksServer());

    // create service
    BLEService *service = serverBLE->createService(UUID_SERVICE_LIGHT);

    // create "characteristic"
    featureLamp = service->createCharacteristic(
                    UUID_CHARATERISTIC_LAMP,
                    BLECharacteristic::PROPERTY_WRITE | 
                    BLECharacteristic::PROPERTY_READ | 
                    BLECharacteristic::PROPERTY_NOTIFY
                );

    // init characteristic
    featureLamp->setValue("off");

    // callback to handle events
    featureLamp->setCallbacks(new CallbackLamp());

    service->start();

    // configure advertising to make the device visible
    BLEAdvertising *servizioAdvertising = BLEDevice::getAdvertising();
    servizioAdvertising->addServiceUUID(UUID_SERVICE_LIGHT); 
    servizioAdvertising->setScanResponse(true);          

    display->clearDisplay();
    display->setCursor(0, 0);
    display->println("Gateway:");
    display->println("Waiting for BLE connection");
    display->display();

    BLEDevice::startAdvertising();

    // *********** setup zigbee protocol ************* //
    IoTBoard::init_spi();
    IoTBoard::init_zigbee();

    zigbee->setAddress(0x01);
    zigbee->setPanId(0x99);
    zigbee->setCoordinator(true);
    zigbee->setChannel(12); // not essential
    
    display->clearDisplay();
    display->setCursor(0, 0);
    display->println("Gateway:");
    display->printf("Zigbee enabled (%04X)\n", zigbee->getAddress());
    display->display();
}

void loopGateway(){
    if (centralConnected && toogleLamp) {
        Serial.println("Toggle Lamp managed by the device");
        toogleLamp = false;
        switchLamp();
    }
}