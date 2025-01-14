/**
 * @file main.cpp
 * @author Alessandro Ferrante (github@alessandroferrante.net)
 * @brief 
 * @version 0.1
 * @date 2024-10-29
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include <WiFi.h>
#include <esp_wifi.h>

enum states {
  GREEN,
  RED,
  YELLOW,
  WARNING
};

states state = WARNING;
// OR RECEIVER MAC Address
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Structure example to send data
// Must match the receiver structure
typedef struct __attribute__((packed)) message {
  char a[32];
  int b;
  float c;
  bool d;
} message;

message myMessage;
esp_now_peer_info_t peerInfo;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void readMacAddress(){
  uint8_t baseMac[6];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  if (ret == ESP_OK) {
    Serial.printf(MACSTR,MAC2STR(baseMac));
  } else {
    Serial.println("Failed to read MAC address");
  }
}

void setup(){
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  WiFi.mode(WIFI_STA);
  WiFi.STA.begin();
  Serial.print("[DEFAULT] ESP32 Board MAC Address: ");
  readMacAddress();

  digitaWrite(LED_RED, LOW);
  digitaWrite(LED_GREEN, LOW);
  digitaWrite(LED_BLUE, LOW);
}


void loop() {
  switch (state)
  {
  case WARNING:
    digitaWrite(LED_RED, !digitalRead(LED_RED));
    digitaWrite(LED_GREEN, !digitalRead(LED_GREEN));
    digitaWrite(LED_BLUE, !digitalRead(LED_BLUE));
    delay(1000);
    break;
  case RED:
    digitaWrite(LED_RED, HIGH);
    digitaWrite(LED_GREEN, LOW);
    break;
  case  GREEN:
    digitaWrite(LED_RED, LOW);
    digitaWrite(LED_GREEN, HIGH);
    break;
  case YELLOW:
    digitaWrite(LED_RED, HIGH);
    digitaWrite(LED_GREEN, LOW);
    delay(3000);
    state = RED;
    messageToSend();
    break; 
  default:

    break;
  }
}