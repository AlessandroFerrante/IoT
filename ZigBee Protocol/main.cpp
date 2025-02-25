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

#include <iot_board.h>

// #define COORDINATOR 1

bool powerOn =  false;
void onBtn1Released(uint8_t time){
  // Serial.println("Pulse pressed");
  zigbee->startPacket();
  zigbee->writeBytes((uint8_t *)&powerOn, sizeof(powerOn));
  zigbee-> sendPacket(0x02, true);
  powerOn = !powerOn;
}

void setup() {
  //IoTBoard::init_//serial();
  IoTBoard::init_buttons();
  IoTBoard::init_leds();
  IoTBoard::init_buttons();
  IoTBoard::init_display();
  display->println("Display enabled");
  display->display();
  buttons->onBtn1Release(onBtn1Released);


  IoTBoard::init_spi();
  IoTBoard::init_zigbee();

  #ifdef COORDINATOR
  zigbee->setAddress(0x01);
  zigbee->setPanId(0x99);
  zigbee->setCoordinator(true);
  #endif

  #ifndef COORDINATOR
  zigbee->setAddress(0x02);
  zigbee->setPanId(0x99);
  #endif

  zigbee->setChannel(12);
  display->printf("Zigbee enabled (%04X)\n", zigbee->getAddress());
  display->display();
  delay(500);


  display->println("SPI (HSPI) enabled");
  display->display();
}

void loop() {
  buttons->update();
 
  #ifndef COORDINATOR
  if (zigbee -> receivePacket()) {
    bool boh;
    zigbee->readBytes((uint8_t *)&boh, sizeof(boh));
  } 
  #endif
}