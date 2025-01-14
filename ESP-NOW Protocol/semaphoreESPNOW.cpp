/**
 * @file semaphoreESPNOW.cpp
 * @author Alessandro Ferrante (github@alessandroferrante.net)
 * @brief 
 * @version 0.1
 * @date 2024-10-29
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>

enum STATE {
    GREEN,
    RED,
    YELLOW,
    WARNING
};

STATE status = WARNING;

typedef struct __attribute((packed)) messagePacket{
    STATE new_status;

} messagePacket;

uint8_t receiverAddress[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

messagePacket messageReceived;
messagePacket messageToSend;
esp_now_peer_info_t peerInfo;

void readMacAddress(){
  uint8_t baseMac[6];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  if (ret == ESP_OK) {
    Serial.printf(MACSTR,MAC2STR(baseMac));
  } else {
    Serial.println("Failed to read MAC address");
  }
}

void onMessageSent(const uint8_t* macDest, esp_now_send_status_t status){
    if(status == ESP_NOW_SEND_SUCCESS){
        Serial.println("Messaggio inviato con successo");
    } else {
        Serial.println("Errore nel trasmettere il messaggio");
    }
}

void onMessageReceived(const uint8_t* macFrom, const uint8_t* bytes, int size){
    memcpy(&messageReceived, bytes, size);
    if(messageReceived.new_status == WARNING) {
        status = GREEN;
        messageToSend.new_status = RED;
        esp_now_send(macFrom, (uint8_t *) &messageToSend, sizeof(messageToSend));
    } else {
        status = messageReceived.new_status;
    }
    
}

void setup() {
    delay(2000);
    Serial.begin(115200);

    pinMode(LED_RED, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_BLUE, OUTPUT);
    WiFi.mode(WIFI_STA);
    readMacAddress();

    if(esp_now_init()!=ESP_OK){
        Serial.println("Failed to initialize protocol, restart in 3 seconds");
        delay(3000);
        esp_restart();
    }

    esp_now_register_send_cb(onMessageSent);
    esp_now_register_recv_cb(onMessageReceived);

    // records the node with which you have to communicate

    memcpy(peerInfo.peer_addr, receiverAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
   
    if( esp_now_add_peer(&peerInfo)!=ESP_OK){
        Serial.println("Unable to add peer");
    }

    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_BLUE, LOW);
    delay(1000);
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_BLUE, HIGH);

}

void loop() {

    switch (status) {
        
    case WARNING:
        digitalWrite(LED_RED, !digitalRead(LED_RED));
        digitalWrite(LED_GREEN, !digitalRead(LED_GREEN));
        digitalWrite(LED_BLUE, HIGH);
        delay(1000);
        messageToSend.new_status = WARNING;
        esp_now_send(receiverAddress, (uint8_t *) &messageToSend, sizeof(messageToSend));
        break;
    
    case RED:
        digitalWrite(LED_RED, LOW);
        digitalWrite(LED_GREEN, HIGH);
        break;
    
    case YELLOW:
        digitalWrite(LED_RED, LOW);
        digitalWrite(LED_GREEN, LOW);
        delay(3000);
        status = RED;
        messageToSend.new_status = GREEN;
        esp_now_send(receiverAddress, (uint8_t *) &messageToSend, sizeof(messageToSend));
        break;

    case GREEN:
        digitalWrite(LED_RED, HIGH);
        digitalWrite(LED_GREEN, LOW);
        delay(3000);
        status = YELLOW;
        messageToSend.new_status = status;
        break;
    
    default:
        break;
    }
}