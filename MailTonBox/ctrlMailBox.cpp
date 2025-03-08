/**
 * @file ctrlMailBox.cpp
 * @author Alessandro Ferrante (github@alessandroferrante)
 * @brief 
 * @version 2.2
 * @date 2025-03-06
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <iot_board.h>
#define TRIG D0
#define ECHO D1
#define ROTARY_CLK D6  // Pin del rotary encoder (Clock)
#define ROTARY_DT D8   // Pin del rotary encoder (Data)

#define SERVO_PIN D9 // Pin di controllo del servo


bool loraFlagReceived = false;
bool loraFlagError = false;
int count = 0;
int count_sent = 0;
bool displayNeedUpdate = false;

// addresses devices for LoRa communication
uint16_t localAddress = 0xFF; 
uint16_t destination = 0x02;       
int theshold = 2;
bool mail_detected = false;
int readings_made = 0;
String last_lora_msg = "New Mail";
String lora_msg = "";
bool lora_priority = false;

// variables for ultrasound sensor
long signal_duration;
float cm;
int initial_distance = 0;
int distance = 0;
int distance_update = 15;


// Rotary encoder
bool mailbox_open = false;
int rotationCount = 6; 
const int CLOSE_THRESHOLD = 2;
const int OPEN_THRESHOLD = -2;
bool wait_rotary = false;
// servo 
bool wait_servo = false;

void IRAM_ATTR rotaryChanged() {
    if (digitalRead(ROTARY_CLK) == digitalRead(ROTARY_DT) && rotationCount < (CLOSE_THRESHOLD+10)) {
        wait_rotary = true;
        rotationCount++;
    } else if(rotationCount > OPEN_THRESHOLD-10) {
        wait_rotary = true;
        rotationCount--;
    }


    // TODO spostarlo nel loop
    if (rotationCount >= CLOSE_THRESHOLD) {
        mailbox_open = false;
    } 
    else if (rotationCount <= OPEN_THRESHOLD) {
        mailbox_open = true;
    }
    wait_rotary = false;
}

// Funzione per generare il segnale PWM per il servo
void writeServo(int angle) {
    wait_servo = true;
    int pulseWidth = map(angle, 0, 180, 1000, 2000); // Mappa l'angolo in un intervallo di larghezza dell'impulso

    // Genera il segnale PWM manualmente
    digitalWrite(SERVO_PIN, HIGH);
    delayMicroseconds(pulseWidth);  // Impulso di larghezza variabile
    digitalWrite(SERVO_PIN, LOW);
    delay(20 - pulseWidth / 1000);  // Resto del ciclo PWM (20 ms)
    wait_servo = false;
}

void onLoRaReceive(int packetSize) {
    // if there's no packet, return
    if (packetSize == 0)
        return; 

    // read packet header bytes:
    uint16_t recipient = lora->read();  // recipient address
    uint16_t sender = lora->read();     // sender address
    byte incomingMsgId = lora->read();  // incoming msg ID
    byte incomingLength = lora->read(); // incoming msg length
    byte receivedChecksum = lora->read(); // read checksum

    String incoming = ""; // payload of packet

    while (lora->available()){          // can't use readString() in callback, so
        incoming += (char)lora->read(); // add bytes one by one
    }
    
    byte calculatedChecksum = 0;
    for (int i = 0; i < incoming.length(); i++) {
        calculatedChecksum ^= incoming[i];
    }
    
    // receivedChecksum != calculatedChecksum 
    if (incomingLength != incoming.length()){ 
        display->clearDisplay();
        display->setCursor(0,0);
        display->println("error: message length or checksum does not match");
        // loraFlagError = true;
    }
    
    count++;
    display->clearDisplay();
    display->setCursor(0,0);
    display->printf("From: %04X\n", sender);
    display->printf("Sent to: %04X\n", recipient);
    display->printf("Message ID: %d\n", incomingMsgId);
    display->printf("Count: %d\n", count);
    display->printf("Message length: %d\n", incomingLength);
    display->printf("Message: %s\n", incoming.c_str());
    display->printf("RSSI: %d\n", lora->packetRssi());
    display->printf("Snr: %02f\n", lora->packetSnr());
    
    lora->receive();

    loraFlagReceived = true;
}

void onLoRaSend(){
    lora_priority = true;
    digitalWrite(LED_GREEN, HIGH);
    last_lora_msg = lora_msg;
    lora->beginPacket();

    lora->write(destination);              // add destination address
    lora->write(localAddress);             // add sender address
    lora->write(count_sent);               // add message ID
    lora->write(lora_msg.length());        // add payload length
    
    byte checksum = 0;
    for (int i = 0; i < lora_msg.length(); i++) {
        checksum ^= lora_msg[i];
    }
    lora->write(checksum); // add checksum
    lora->print(lora_msg); // add payload

    lora->endPacket(true); // true = async / non-blocking mode
    delay(100);

    count_sent++;
    lora->receive();

    loraFlagReceived = false;

    digitalWrite(LED_GREEN, LOW);
    lora_priority = false;
}

void onBtn1Released(uint8_t pinBtn){
}

int measuresDistance() {
    digitalWrite(TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG, LOW);
    
    int signal_duration = pulseIn(ECHO, HIGH, 30000);
    int cm = signal_duration / 58;
    // for the range
    if(cm > 1000) cm = 1;
    return (cm > 0 && cm < 100) ? cm : 100;
}

int measuresAverageDistance() {
    long sum = 0;
    int readings = 20;
    for (int i = 0; i < readings; i++) {
        int distance = measuresDistance();
        sum += distance;
        delay(10);
    }
    return sum / readings;
}

void setup() {
    IoTBoard::init_serial();
    IoTBoard::init_buttons();
    IoTBoard::init_leds();
    IoTBoard::init_display();
    display->println("Display enabled");
    display->display();

    IoTBoard::init_spi();
    display->println("SPI (HSPI) enabled");
    display->display();

    if (IoTBoard::init_lora()) {
        lora->onReceive(onLoRaReceive);
        lora->onTxDone(onLoRaSend);
        lora->receive();
        display->println("LoRa enabled");
    } else {
        digitalWrite(LED_RED, HIGH);
        display->printf("LoRa Error");
    }
    display->display();

    buttons->onBtn1Release(onBtn1Released);

    pinMode(ROTARY_CLK, INPUT_PULLUP);
    pinMode(ROTARY_DT, INPUT_PULLUP);
    //pinMode(ROTARY_SW, INPUT_PULLUP);
    pinMode(SERVO_PIN, OUTPUT);
    attachInterrupt(digitalPinToInterrupt(ROTARY_CLK), rotaryChanged, CHANGE);

    pinMode(TRIG, OUTPUT);
    pinMode(ECHO, INPUT);
    initial_distance = measuresAverageDistance();
    delay(500);
}

void loop() {
    buttons->update();

    if(displayNeedUpdate){
        displayNeedUpdate = false;
        display->display();
    }
    
    if (!lora_priority){
        distance = measuresAverageDistance();
        readings_made++;
    
        
        if (readings_made >= distance_update) {
            initial_distance = measuresAverageDistance();
            readings_made = 0;
        }
    } 
    
    if (!mailbox_open){
        writeServo(90);
        delay(10);
        if (!wait_rotary && !mail_detected && abs(distance - initial_distance) > theshold){        
            display->clearDisplay();
            display->setCursor(0,0);
            display->println("Letter detected");
            display->display();
            lora_msg = "New Mail";
            onLoRaSend();
            mail_detected = true;
            delay(1000);
        } else if (mail_detected && abs(distance - initial_distance) <= theshold) {
            mail_detected = false; // reset when the letter is removed
        }
    }
    static bool message_sent = false;
    if (mailbox_open) {
        writeServo(-30); 
        delay(100);
        if (!mail_detected && !message_sent) {
            // Send the message only if the mailbox is open and the mail is taken (or the distance is the initial one)
            lora_msg = "Mailbox Opened";
            onLoRaSend();
            delay(100);
            message_sent = true;
        }
        if (mail_detected && abs(distance - initial_distance) <= theshold) {
            mail_detected = false; // reset when the letter is removed
        }
        
    } else {
        message_sent = false; // reset when the mailbox is closed
    }

    if(loraFlagReceived){
        lora_msg = last_lora_msg;
        delay(100);
        onLoRaSend();
        loraFlagReceived = false;
        display->display();
    }

    if (loraFlagError){
        loraFlagError = false;
        display->display();
    }

    display->clearDisplay();
    display->setCursor(0,0);
    display->print(distance);
    display->print(" cm");
    display->setCursor(0,8);
    display->print("Rotation Count: ");
    display->print(rotationCount);
    display->setCursor(0,16);
    display->println(mailbox_open ? "MAILBOX OPEN" : "MAILBOX CLOSE");
    display->display();

    delay(1);
}