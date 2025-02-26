/**
 * @file lamp.h
 * @author Alessandro Ferrante (github@alessandroferrante.net)
 * @brief 
 * @version 0.1
 * @date 2024-12-18
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "lamp.h"

bool powerSwitch = false;
int ColorArray[3] = {LED_GREEN, LED_RED, LED_YELLOW};
int colors = 0;

void changeColor();
void onBtn2Release();
void onBtn2Pressed();

void onBtn2Release() {
    detachInterrupt(BTN_2);
    attachInterrupt(BTN_2, onBtn2Pressed, FALLING);
    changeColor();
}

void onBtn2Pressed() {
    detachInterrupt(BTN_2);
    attachInterrupt(BTN_2, onBtn2Release, RISING);
}

void updateLamp(){
    if (powerSwitch)
        digitalWrite(ColorArray[colors], HIGH);
    else
        digitalWrite(ColorArray[colors], LOW);    
}

void changeColor(){
  colors++;
  if(colors == 3)
    colors = 0;
}

void setupLamp() {
  display->println("LampZigbee");
  pinMode(BTN_2, INPUT_PULLUP);
  attachInterrupt(BTN_2, onBtn2Pressed, FALLING);
  
  IoTBoard::init_spi();
  IoTBoard::init_zigbee();
  // ? zigbee->setCoordinator(false); // not essential
  zigbee->setChannel(12);

  zigbee->setAddress(0x02);
  zigbee->setPanId(0x99);
}

void loopLamp() {
  if (zigbee -> receivePacket())
    zigbee->readBytes((uint8_t *)&powerSwitch, sizeof(powerSwitch));
  
  updateLamp();
}