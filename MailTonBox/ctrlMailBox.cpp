/**
 * @file ctrlMailBox.cpp
 * @author Alessandro Ferrante (github@alessandroferrante)
 * @brief 
 * @version 3.0
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

#define SERVO_PIN A6 // Pin di controllo del servo

String CTRLMAILBOX_KEY = "VSJ5KNVS903NJ7N12NN";
String CTRLMAILBOX_NAME = "CMBX1";
String MAILTON_KEY = "00BKFWR39FN48FN40GM30DM69GJ";

bool loraFlagReceived = false;
int count = 0;
int count_sent = 0;
bool displayNeedUpdate = false;

// for LoRa communication
uint16_t localAddress = 0x01; 
uint16_t destination = 0x02;       
int theshold = 2;
bool mail_detected = false;
int readings_made = 0;
String last_lora_msg = "New Mail";
String lora_msg = "";
bool lora_priority = false;
String last_message_received;

// variables for ultrasound sensor
long signal_duration;
float cm;
int initial_distance = 0;
int distance = 0;
int distance_update = 15;

// Rotary encoder
bool mailbox_open = false;
int rotationCount = 5; 
const int CLOSE_THRESHOLD = 15;
const int OPEN_THRESHOLD = -15;
bool wait_rotary = false;
// servo
bool wait_servo = false;
bool servo_open = false;

unsigned long lastSendTime = 0;
const unsigned long waitTime = 15000; // Tempo di attesa per la risposta (5 secondi)
bool waitingForResponse = false;

void IRAM_ATTR rotaryChanged() {
    wait_rotary = true;
    if (digitalRead(ROTARY_CLK) == digitalRead(ROTARY_DT)) {
        mailbox_open = false;
    } else {
        mailbox_open = true;
    }
    wait_rotary = false;
}

// funzione per generare il segnale PWM per il servo
void writeServo(int angle) {
    wait_servo = true;
    // per mappare l'angolo in un intervallo di larghezza dell'impulso
    int pulseWidth = map(angle, 0, 180, 1000, 2000); 
    
    // Genera il segnale PWM manualmente
    digitalWrite(SERVO_PIN, HIGH);
    delayMicroseconds(pulseWidth);  // Impulso di larghezza variabile
    digitalWrite(SERVO_PIN, LOW);
    delay(20 - pulseWidth / 1000);  // Resto del ciclo PWM (20 ms)

    if(angle == 90) servo_open = false;
    else servo_open = true;
    wait_servo = false;
}

bool isAckMessage(const String& message) {
    return message == "ACK";
}

// function to extract values ​​from a "key = value" string
String extractValue(String message, String key) {
    int startIndex = message.indexOf(key + "=");
    if (startIndex == -1) return "";
    startIndex += key.length() + 1;
    int endIndex = message.indexOf(";", startIndex);
    if (endIndex == -1) endIndex = message.length();
    return message.substring(startIndex, endIndex);
}

// function to receive messages Lora
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
    if (incomingLength != incoming.length() || receivedChecksum != calculatedChecksum){ 
        display->clearDisplay();
        display->setCursor(0,0);
        display->println("error: message length or checksum does not match");
        return;
    }

    // extract name, key and data
    String senderName = extractValue(incoming, "NAME");
    String senderKey = extractValue(incoming, "MAILTON_KEY");
    String receiverKey = extractValue(incoming, "CTRLMAILBOX_KEY");
    String data = extractValue(incoming, "DATA");

    // Controllo autenticazione
    //Preferences preferences;
    //preferences.begin("configuration", true);
    //String expectedSenderKey = preferences.getString("CTRLMAILBOX_KEY", "");
    //preferences.end();
    String expectedSenderKey = MAILTON_KEY; // todo da salvare in prefereces
    String expectedReceiverKey = CTRLMAILBOX_KEY;

    if (senderKey != expectedSenderKey || receiverKey != expectedReceiverKey) {
        Serial.println("Messaggio NON autenticato! Ignorato.");
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
    display->printf("Message: %s\n", data.c_str());
    display->printf("RSSI: %d\n", lora->packetRssi());
    display->printf("Snr: %02f\n", lora->packetSnr());
    
    lora->receive();
    last_message_received = data.c_str();
    loraFlagReceived = true; // set the reception flag
}

void onLoRaSend() {
}

void sendMessageLoRa() {
    lora_priority = true;
    digitalWrite(LED_GREEN, HIGH);
    last_lora_msg = lora_msg;
    
    //Preferences preferences;
    //preferences.begin("lora", true);
    String senderName = CTRLMAILBOX_NAME;
    String senderKey = CTRLMAILBOX_KEY;
    String receiverKey = MAILTON_KEY;

    //String senderKey = preferences.getString("MAILTON_KEY", "");  
    //preferences.end();

    // Componi il messaggio nel formato: NAME=XYZ;KEY=ABC;DATA=Messaggio
    String messageToSend = "NAME=" + senderName + ";CTRLMAILBOX_KEY=" + senderKey + ";MAILTON_KEY=" + receiverKey + ";DATA=" + lora_msg;

    lora->beginPacket();

    lora->write(destination);              // add destination address
    lora->write(localAddress);             // add sender address
    lora->write(count_sent);               // add message ID
    lora->write(messageToSend.length());   // add payload length

    byte checksum = 0;
    for (int i = 0; i < messageToSend.length(); i++) {
        checksum ^= messageToSend[i];
    }
    lora->write(checksum); // add checksum
    lora->print(messageToSend); // add payload

    lora->endPacket(true); // true = async / non-blocking mode
    delay(100);

    count_sent++;
    lora->receive();

    loraFlagReceived = false;
    waitingForResponse = true; // Imposta il flag di attesa
    lastSendTime = millis(); // Registra il tempo di invio

    digitalWrite(LED_GREEN, LOW);
    lora_priority = false;
}


void onBtn1Released(uint8_t pinBtn){
    lora_msg = "AAAAAAAA";
    sendMessageLoRa();
}

void onBtn2Released(uint8_t pinBtn){
    mailbox_open = !mailbox_open;
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
    buttons->onBtn2Release(onBtn2Released);

    pinMode(ROTARY_CLK, INPUT_PULLUP);
    pinMode(ROTARY_DT, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(ROTARY_CLK), rotaryChanged, CHANGE);
    pinMode(SERVO_PIN, OUTPUT);

    pinMode(TRIG, OUTPUT);
    pinMode(ECHO, INPUT);
    initial_distance = measuresAverageDistance();
    delay(500);
}

void loop() {
    buttons->update();
    
    // | ******************** Restart LoRa *************************
    /*if(!wait_rotary && !wait_servo && !servo_open && !mailbox_open){
        lora_priority = true;
        if(!lora->isConnected()){
            digitalWrite(LED_RED, HIGH);
            display->printf("LoRa Error");
            
            // !!!!! Implies the reset esp32
            if (IoTBoard::init_lora()) {
                lora->onReceive(onLoRaReceive);
                lora->onTxDone(onLoRaSend);
                lora->receive();
                display->println("LoRa enabled");
            } else {
                digitalWrite(LED_RED, HIGH);
                display->printf("LoRa Error");
            }
            
        } else if(lora->isConnected()){
            digitalWrite(LED_RED, LOW);
        }
        displayNeedUpdate = true;
        lora_priority = false;
    }*/
    
    delay(1);
    
    if(displayNeedUpdate){
        displayNeedUpdate = false;
        display->display();
    }
    
    if (!lora_priority && !wait_rotary && !wait_servo){
        distance = measuresAverageDistance();
        readings_made++;       
        if (readings_made >= distance_update) {
            initial_distance = measuresAverageDistance();
            readings_made = 0;
        }
    } 
    
    if (!lora_priority && !mailbox_open && !wait_servo && !wait_rotary && !mail_detected && abs(distance - initial_distance) > theshold){        
        display->clearDisplay();
        display->setCursor(0,0);
        display->println("Letter detected");
        display->display();
        lora_msg = "New Mail";
        noInterrupts();
        sendMessageLoRa();
        interrupts();
        mail_detected = true;
        delay(1000);
    } else if (mail_detected && abs(distance - initial_distance) <= theshold) {
        mail_detected = false; // reset when the letter is removed
    }  

    static bool message_sent = false;
    
    if (!lora_priority && mailbox_open && servo_open && !wait_rotary && !wait_servo && !mail_detected && !message_sent) {
        // Send the message only if the mailbox is open and the mail is taken (or the distance is the initial one)
        lora_msg = "Mailbox Opened";
        noInterrupts();
        sendMessageLoRa();
        interrupts();
        delay(100);
        message_sent = true;
    }else if (mail_detected && abs(distance - initial_distance) <= theshold) {
        mail_detected = false; // reset when the letter is removed
    }
    if(!mailbox_open)
        message_sent = false; // reset when the mailbox is closed


    // if the waiting time (waitTime=15s) has expired, retransmitte message, 
    if (waitingForResponse && (millis() - lastSendTime > waitTime)) {
        sendMessageLoRa();
    }
    // if receives an answer, check if he is an ACK, otherwise postpone
    if(loraFlagReceived){
        lora_msg = last_lora_msg;
        
        if (isAckMessage(last_message_received)) {
            waitingForResponse = false; // reset of the waiting flag
        } else {
            delay(10);
            noInterrupts();
            sendMessageLoRa();
            interrupts();
            delay(100);
        }
        
        loraFlagReceived = false;       
        display->display();
        delay(1000);
    }

    
    /*if (rotationCount >= CLOSE_THRESHOLD-5) {
        writeServo(90);
        mailbox_open = false;
    } 
    else if (rotationCount <= OPEN_THRESHOLD+5) {
        writeServo(-30);
        mailbox_open = true;
    }*/
    
    //    delay(1000);
    
    if (!mailbox_open){
        writeServo(90);
        delay(10);
    }else {
        writeServo(-30); 
        delay(10);
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