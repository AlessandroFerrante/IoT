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
#include "BLEbtn.h"


// variables shared with main

BLEScan *bleScan;
BLEAddress *pServerAddress; 
BLERemoteCharacteristic *remoteFeature; 
bool connected = false;
bool deviceFound = false;

void switchLamp();

void callbackNotify(BLERemoteCharacteristic *notify, uint8_t *pData, size_t length, bool isNotify) {
    String s = String((char *)pData, length);
    Serial.println(s);
    if (s == "on") {
    } else {
    }
}

// class to handle visible device events
class CallbacksAdvertising : public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) override {
        
        Serial.println(advertisedDevice.toString().c_str());

        if (advertisedDevice.haveName() && advertisedDevice.getName() == "gateway_lamp") {
           
            display->println("Device Found");
            display->display();

            deviceFound = true;
            bleScan->stop(); 
            pServerAddress = new BLEAddress(advertisedDevice.getAddress());
        }
    }
};

// to connect to the device (gateway)
void connectToLamp() {
    display->clearDisplay();
    display->setCursor(0, 0);
    display->println("Server:");
    display->println("Connecting");
    display->display();

    BLEClient *bleClient = BLEDevice::createClient();
    bleClient->connect(*pServerAddress);

    // get the remote service with the UUID
    BLERemoteService *remoteService = bleClient->getService(UUID_SERVICE_LIGHT);

    // retrieve the remote feature and register a callback for notifications
    remoteFeature = remoteService->getCharacteristic(UUID_CHARATERISTIC_LAMP);
    remoteFeature->registerForNotify(callbackNotify, true, false);

    // update the connection status
    connected = true;

    display->println("Gateway connected");
    display->display();
}

void setupBLEbtn() {
    BLEDevice::init("");
    display->println("Server:");
    display->println("Searching for the device");
    display->display();

    // Configure BLE scanning
    bleScan = BLEDevice::getScan();
    bleScan->setAdvertisedDeviceCallbacks(new CallbacksAdvertising()); 
    bleScan->setActiveScan(true);
    bleScan->setInterval(100);
    bleScan->setWindow(90);
    bleScan->start(5000, true); 

    display->clearDisplay();
    display->setCursor(0, 0);
    display->println("Server:");
    display->display();

}

void loopBLEbtn(){
    if (deviceFound && !connected) {
        Serial.println("Attempt to connect to the found deviceo");
        connectToLamp();
    } else if (toogleLamp) {
        Serial.println("Lamp sent");
        toogleLamp = false;
        switchLamp();
    }
}