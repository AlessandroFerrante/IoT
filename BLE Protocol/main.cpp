/**
 * @file main.cpp
 * @author Alessandro Ferrante (github@alessandroferrante.net)
 * @brief 
 * @version 0.1
 * @date 2024-12-17
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "peripheral.h"
#include "server.h"

bool toToggle = false;
bool statusLedVerde = false;

void onBtn1Release();
void onBtn1Pressed();

// Release Management of the BTN_1 button
void onBtn1Release() {
    Serial.println("BTN_1 rilasciato");
    detachInterrupt(BTN_1);
    attachInterrupt(BTN_1, onBtn1Pressed, FALLING);
    toToggle = true;
}

// BTN_1 button pressure management
void onBtn1Pressed() {
    Serial.println("BTN_1 premuto");
    detachInterrupt(BTN_1);
    attachInterrupt(BTN_1, onBtn1Release, RISING);
}

// Function to alternate the status of the green LED
void toggleLed() {
    #ifndef BLE_PERIFERICA
    statusLedVerde = !statusLedVerde;
    Serial.print("Stato LED Verde: ");
    Serial.println(statusLedVerde ? "on" : "off");
    if (statusLedVerde) {
        characteristic->writeValue("on");
    } else {
        characteristic->writeValue("off");
    }
    #endif

    #ifdef BLE_PERIFERICA
    if (caratteristicaLedVerde->getValue() == "on") {
        caratteristicaLedVerde->setValue("off");
        digitalWrite(LED_GREEN, LOW);
        Serial.println("LED Verde spento");
    } else {
        caratteristicaLedVerde->setValue("on");
        digitalWrite(LED_GREEN, HIGH);
        Serial.println("LED Verde acceso");
    }
    caratteristicaLedVerde->notify();
    #endif
}

// Initial setup of the system
void setup() {
    IoTBoard::init_serial();
    IoTBoard::init_leds();
    IoTBoard::init_display();
    Serial.println("Initialized system");
    display->println("Display enabled");
    display->display();

    pinMode(BTN_1, INPUT_PULLUP);
    attachInterrupt(BTN_1, onBtn1Pressed, FALLING);

    #ifdef BLE_PERIFERICA
    Serial.println("Configurazione come periferica BLE");
    setupPeriferica();
    #endif

    #ifndef BLE_PERIFERICA
    Serial.println("Configurazione come dispositivo centrale BLE");
    setupServer();
    #endif
}

// Main cycle of the system
void loop() {
    #ifndef BLE_PERIFERICA
    if (deviceFound && !connected) {
        Serial.println("Tentativo di connessione al dispositivo trovato");
        connectToLampada();
    } else if (toToggle) {
        Serial.println("Toggle LED inviato");
        toToggle = false;
        toggleLed();
    }
    #endif

    #ifdef BLE_PERIFERICA
    if (centralConnected && toToggle) {
        Serial.println("LED toggle managed by the peripheral.");
        toToggle = false;
        toggleLed();
    }
    #endif
}
