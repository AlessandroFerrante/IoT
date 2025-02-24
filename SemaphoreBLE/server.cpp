/**
 * @file server.h
 * @author Alessandro Ferrante (github@alessandroferrante.net)
 * @brief 
 * @version 0.1
 * @date 2024-11-26
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef SERVER_H
#define SERVER_H

#include "server.h"

#define MAX_SLAVES 3
int countSlave = 0;

/*
// Global variables shared with the main
extern bool connected; // Ble connection status
extern bool deviceFound; // Flag indicating if the device has been found
extern BLEScan *bleScan; // object for the BLE scan
extern BLEAddress *pServerAddress; // Ble Server address
extern BLERemoteCharacteristic *characteristic; // Remote feature for interactions
*/
int stringToSemaphoreState(const String& value);

typedef struct{
    BLEAddress *address;
    BLERemoteCharacteristic *characteristic;
    bool connected;
} SemaphoreNode;

list<SemaphoreNode> peripheralsList;
list<SemaphoreNode>::iterator it;

/**
 * @param semaphore master traffic-light color
 */
int semaphore = YELLOW;
int semaphoreConnected = 0;

void updateSemaphore(String color);

void connectToSemaphore();

void updateSemaphore(String color) {
    int counter = 0;
    for (const auto& node : peripheralsList) {
        if (node.connected) { // Check if the device is connected
            if (counter < floor(peripheralsList.size() / 2))
                node.characteristic->writeValue(semaphore);
            else
                node.characteristic->writeValue(color);
        } else {
            Serial.println("Un dispositivo non è connesso"); 
            //  TODO controllare...
        }
        counter++;
    }
}

// Callbacks for the notification of changes in the remote feature
void callbackNotify(BLERemoteCharacteristic *carNotificata, uint8_t *pData, size_t length, bool isNotify) {
    String s = String((char *)pData, length);
    Serial.print("Notifica ricevuta: ");
    int slaveSemaphore = stringToSemaphoreState(s);
    // used for synchronization
    switch (slaveSemaphore) {
    case RED:
        semaphore = GREEN;
        break;
    case YELLOW:
        semaphore = GREEN;
        break;
    case GREEN:
        semaphore = RED;
        break;
    default:
        break;
    }
}

// manage advertised device events during the scan
class CallbacksAdvertising : public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) override {
        // Print on the serial the information of the device found
        Serial.println(advertisedDevice.toString().c_str());

        // Check if the advertised device has the expected name
        if (advertisedDevice.haveName() && advertisedDevice.getName() == "semaphore") {
            display->clearDisplay();
            display->setCursor(0, 0);
            display->println("Dispositivo Trovato");
            display->display();
            SemaphoreNode s;
            s.address = new BLEAddress(advertisedDevice.getAddress());
            s.connected = false;
            peripheralsList.push_back(s);
            display->printf("Dispositivo Inserito in Lista: %d/%d", peripheralsList.size(), MAX_SLAVES);
            display->display();
            
            if (peripheralsList.size() == MAX_SLAVES) {
                bleScan->stop();
                it = peripheralsList.begin();
            }
        }
    }
};

// Function to connect to the device
void connectToSemaphore(list<SemaphoreNode>::iterator peripheral) {
    Serial.println("Tentativo di connessione al dispositivo trovato");
    // Update the display to show the state
    display->clearDisplay();
    display->setCursor(0, 0);
    display->printf("Collegamento in corso %d/%d", semaphoreConnected + 1, MAX_SLAVES);
    display->display();

    // Create a BLE client and try to connect to the server
    BLEClient *bleClient = BLEDevice::createClient();
    bleClient->connect(*peripheral->address);

    // Recovers the remote service using UUID
    BLERemoteService *remoteService = bleClient->getService(UUID_SERVICE_LIGHT);
    // Recovers the remote feature and records a callback for notifications
    characteristic = remoteService->getCharacteristic(UUID_CHARATERISTIC_SEMAPHORE);
    
    characteristic->registerForNotify(callbackNotify, true, false);

    // Update the state of the peripheral connection
    peripheral->connected = true;

    display->clearDisplay();
    display->setCursor(0, 0);
    display->println("Dispositivo collegato");
    display->display();
}

// Function to configure the BLE central device
void setupServer() {
    // Initialize the BLE device without a specific name
    BLEDevice::init("");

    // Show a message on the display
    display->println("Searching for the device");
    display->display();

    // Configure the BLE scan
    bleScan = BLEDevice::getScan();
    bleScan->setAdvertisedDeviceCallbacks(new CallbacksAdvertising()); // Assign callback
    bleScan->setActiveScan(true); // Activate the active scan
    bleScan->setInterval(100); // Scan interval (MS)
    bleScan->setWindow(90); // Duration of the scan window (MS)
    bleScan->start(5000, true); // Start the scan for 5 seconds
}

void loopServer(){
    if (MAX_SLAVES == peripheralsList.size()){
        if (semaphoreConnected < peripheralsList.size()) {
            if (it->connected == false){
                connectToSemaphore(it);
                it++;
                semaphoreConnected++;
            }
        }
        else
        {
            display->clearDisplay();
            display->setCursor(0, 0);
            display->println("Tutti i dispositivi ssssssssssono collegati");
            display->display();

            switch (semaphore) {
            case GREEN:
                digitalWrite(LED_GREEN, HIGH);
                digitalWrite(LED_YELLOW, LOW);
                digitalWrite(LED_RED, LOW);
                updateSemaphore("RED");
                delay(1000);
                semaphore = YELLOW;
                break;
            case YELLOW:
                digitalWrite(LED_GREEN, LOW);
                digitalWrite(LED_YELLOW, HIGH);
                digitalWrite(LED_RED, LOW);
                updateSemaphore("RED");
                delay(1000);
                semaphore = RED;
                break;
            case RED:
                digitalWrite(LED_GREEN, LOW);
                digitalWrite(LED_YELLOW, LOW);
                digitalWrite(LED_RED, HIGH);
                semaphore = GREEN;
                updateSemaphore("GREEN");
                delay(1000);
                break;
            default:
                break;
            }
        }
    }
}

#endif
