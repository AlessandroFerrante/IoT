/**
 * @file ctrlMailBox.cpp
 * @author Alessandro Ferrante (github@alessandroferrante)
 * @brief 
 * @version 1.5.0
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

int count = 0;
int count_sent = 0;
bool displayNeedUpdate = false;

// addresses devices for LoRa communication
uint16_t localAddress = 0xFF; 
uint16_t destination = 0x02;       
int theshold = 1;
bool mail_detected = false;
int readings_made = 0;

// variables for ultrasound sensor
long signal_duration;
float cm;
int initial_distance = 0;
int initial_distance_saved = 0;


// Rotary encoder
volatile bool mailbox_open = false;
volatile int rotationCount = 26; 
const int OPEN_THRESHOLD = 25;
const int CLOSE_THRESHOLD = -25;

void IRAM_ATTR rotaryChanged() {
    if (digitalRead(ROTARY_CLK) == digitalRead(ROTARY_DT)) {
        rotationCount +=1;
    } else {
        rotationCount -=1;
    }

    if (rotationCount >= OPEN_THRESHOLD) {
        mailbox_open = false;
    } 
    else if (rotationCount <= CLOSE_THRESHOLD) {
        mailbox_open = true;
    }
}

// Funzione per generare il segnale PWM per il servo
void writeServo(int angle) {
    int pulseWidth = map(angle, 0, 180, 1000, 2000); // Mappa l'angolo in un intervallo di larghezza dell'impulso

    // Genera il segnale PWM manualmente
    digitalWrite(SERVO_PIN, HIGH);
    delayMicroseconds(pulseWidth);  // Impulso di larghezza variabile
    digitalWrite(SERVO_PIN, LOW);
    delay(20 - pulseWidth / 1000);  // Resto del ciclo PWM (20 ms)
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

    String incoming = ""; // payload of packet
    // add bytes one by one
    while (lora->available())
        incoming += (char)lora->read(); 
    
    if (incomingLength != incoming.length()){ 
        display->println("error: message length does not match length");
        return; 
    }

    count++;
    display->clearDisplay();
    display->setCursor(0,0);
    display->printf("From: %04X\n", sender);
    display->printf("Sent to: %04X\n", recipient);
    display->printf("Message ID: %d\n", incomingMsgId);
    display->printf("Count: %d\n", count);
    display->printf("Message length: %d\n", incomingLength);
    display->printf("Message: %s\n", incoming);
    display->printf("RSSI: %d\n", lora->packetRssi());
    display->printf("Snr: %02f\n", lora->packetSnr());
    displayNeedUpdate = true;
}

void onLoRaSend() {
}

void sendMessageLoRa(){
    digitalWrite(LED_GREEN, HIGH);
    String lora_msg = "Posta arrivata " + String(count_sent);
    lora->beginPacket();

    lora->write(destination);              // add destination address
    lora->write(localAddress);             // add sender address
    lora->write(count_sent);               // add message ID
    lora->write(lora_msg.length());        // add payload length
    lora->print(lora_msg); // add payload

    lora->endPacket(true); // true = async / non-blocking mode
    count_sent++;
    lora->receive();

    delay(100);
    digitalWrite(LED_GREEN, LOW);
}

void onBtn1Released(uint8_t pinBtn){
    sendMessageLoRa();
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
    
    int distance = measuresAverageDistance();

    if (!mailbox_open){
        if (!mail_detected && abs(distance - initial_distance) > theshold){  

            display->clearDisplay();
            display->setCursor(0,0);
            display->println("Letter detected");
            display->display();
            
            sendMessageLoRa();
            mail_detected = true;
            delay(100);
        } else if (mail_detected && abs(distance - initial_distance) <= theshold) {
            mail_detected = false; // reset when the letter is removed
        }
    }
    if (mailbox_open) {
        if (mail_detected && abs(distance - initial_distance) <= theshold) {
            mail_detected = false; // reset when the letter is removed
        }
    }
    
     // Controlla l'apertura/chiusura della cassetta della posta e muove il servomotore di conseguenza
    if (mailbox_open) {
        writeServo(-30);  // Muove il servo a 90°
    } else {
        writeServo(90);   // Muove il servo a 0°
    }

    display->clearDisplay();
    display->setCursor(5,0);
    display->print(distance);
    display->print("cm");

    display->println("Rotation Count: ");
    display->print(rotationCount);
    display->println(" | Mailbox Open: ");
    display->println(mailbox_open);
    display->display();

    delay(1);
}
