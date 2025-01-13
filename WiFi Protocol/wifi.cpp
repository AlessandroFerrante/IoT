/**
 * @file wifi.cpp
 * @author Alessandro Ferrante (github@alessandroferrante.net)
 * @brief Implements the WiFi protocol for an Arduino-based project.
 * 
 * This file contains the implementation of a WiFi access point using the Arduino framework.
 * It sets up the WiFi mode, handles client connections, and provides feedback through LED indicators.
 * 
 * The code initializes the WiFi module in Access Point (AP) mode and sets up event handling for client connections.
 * It also configures three LEDs to indicate the status of the WiFi mode:
 * - Blue LED: Indicates that the device is in AP mode.
 * - Red LED: Indicates that the device is in Station (STA) mode.
 * - Green LED: Reserved for future use.
 * 
 * The `setup` function initializes the serial communication, configures the WiFi mode, and sets up the LEDs.
 * The `loop` function is currently empty and can be used for additional functionality.
 * 
 * Functions:
 * - `on_connected_station`: Callback function that handles the event when a client connects to the AP.
 * - `setup`: Initializes the WiFi module, sets up the LEDs, and configures the WiFi mode.
 * - `loop`: Main loop function, currently empty.
 * 
 * Usage:
 * - Ensure that the correct SSID and password are set for the AP mode.
 * - Upload the code to an Arduino-compatible board with WiFi capabilities.
 * - Monitor the serial output for connection status and client information.
 * @version 0.1
 * @date 2024-10-24
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <Arduino.h>
#include <WiFi.h>

bool statoLed = false;
void on_connected_station(WiFiEvent_t event,WiFiEventInfo_t info ){
    Serial.println("connessione\n");

    Serial.print("Mac address of the client: ");
    Serial.printf(MACSTR +'\n', MAC2STR(info.wifi_ap_staconnected.mac));
    Serial.println(IPAddress(info.wifi_ap_staconnected.aid));
}

void setup(){
    delay(2000);
    Serial.begin(115200);
    WiFi.disconnect(true);  
    pinMode(LED_RED,OUTPUT);
    pinMode(LED_GREEN,OUTPUT);
    pinMode(LED_BLUE,OUTPUT);

    digitalWrite(LED_BLUE,HIGH);
    digitalWrite(LED_RED,HIGH);
    digitalWrite(LED_GREEN,HIGH);

    WiFi.mode(WIFI_AP);

    switch (WiFi.getMode())
    {
    case WIFI_AP:
        digitalWrite(LED_BLUE,LOW);
        WiFi.softAP("YourSSID" , "YourPassword");
        WiFi.onEvent(on_connected_station, ARDUINO_EVENT_WIFI_AP_STACONNECTED);
        Serial.println("Start completed access point");    
        break;
    
    case WIFI_STA:
        digitalWrite(LED_RED,LOW);
        Serial.println("Start completed Station");

        Serial.println("Reti Scan Start...");
        int numReti = WiFi.scanNetworks();
        for (int i=0; i<numReti; i++){
            Serial.printf("SSID: %s \t RSSI: %d ", WiFi.SSID(1), WiFi.RSSI(i));
        }
        break;
    
    }
    
}
void loop(){

}