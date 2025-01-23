/**
 * @file server.cpp
 * @author Alessandro Ferrante (github@alessandroferrante.net)
 * @brief 
 * @version 0.1
 * @date 2024-12-17
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "server.h"

// Global variables shared with the Main
bool connected = false; // BLE connection status
bool deviceFound = false; // Flag indicating if the device was found
BLEScan *bleScan; // object for the BLE scan
BLEAddress *pServerAddress; // Ble Server address
BLERemoteCharacteristic *characteristic; // Remote feature for interactions


// Callback per la notifica di cambiamenti nella caratteristica remota
void callbackNotifica(BLERemoteCharacteristic *carNotificata, uint8_t *pData, size_t length, bool isNotify) {
    String s = String((char *)pData, length);
    Serial.print("Notifica ricevuta: ");
    Serial.println(s);
    if (s == "on") {
        digitalWrite(LED_GREEN, HIGH);
    } else {
        digitalWrite(LED_GREEN, LOW);
    }
}

// Class to manage devices of devices advertised during scanning
class CallbacksAdvertising : public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) override {
        // Print on the serial the information of the device found
        Serial.println(advertisedDevice.toString().c_str());

        // check if the advertised device has the expected name
        if (advertisedDevice.haveName() && advertisedDevice.getName() == "lampada_philips") {
            // Show a message on the display
            display->println("Dispositivo Trovato");
            display->display();

            // Update the flag and save the device address
            deviceFound = true;
            bleScan->stop(); // stop the scan
            pServerAddress = new BLEAddress(advertisedDevice.getAddress()); // save the address
        }
    }
};

// Function to connect to the Ble device "Philips lamp"
void connectToLampada() {
    // Update the display to show the state
    display->clearDisplay();
    display->setCursor(0, 0);
    display->println("Collegamento in corso");
    display->display();

    // Create a BLE client and tries to connect to the server
    BLEClient *bleClient = BLEDevice::createClient();
    bleClient->connect(*pServerAddress);

    // recover the remote service using UUID
    BLERemoteService *servizioRemoto = bleClient->getService(UUID_SERVICE_LIGHT);

    // recover the remote feature and records a callback for notifications
    characteristic = servizioRemoto->getCharacteristic(UUID_CHARATERISTIC_LED_GREEN);
    characteristic->registerForNotify(callbackNotifica, true, false);

    // Update the state of the connection
    connected = true;

    // Show a message on the display
    display->println("Dispositivo collegato");
    display->display();
}

// Function to configure the central BLE device
void setupServer() {
    // Initialize the BLE device without a specific name
    BLEDevice::init("");

    // Show a message on the display
    display->println("Ricerca del dispositivo");
    display->display();

    // Configure the BLE scan
    bleScan = BLEDevice::getScan();
    bleScan->setAdvertisedDeviceCallbacks(new CallbacksAdvertising()); // Assign callback
    bleScan->setActiveScan(true); // Activate the active scan
    bleScan->setInterval(100); // Scan interval (MS)
    bleScan->setWindow(90); // Duration of the Scan Window (MS)
    bleScan->start(5000, true); // Start the scan for 5 seconds
}

