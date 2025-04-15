/**
 * @file ctrlMailBox.cpp
 * @author Alessandro Ferrante (github@alessandroferrante.net)
 * @brief Firmware for managing a smart mailbox with LoRa communication, WiFi configuration, and sensor integration.
 * 
 * This firmware is designed to control a smart mailbox system that integrates multiple functionalities:
 * - LoRa communication for long-range data transmission.
 * - WiFi configuration for local network setup and web-based management.
 * - Ultrasonic sensor for detecting the presence of mail.
 * - Rotary encoder for manual control of the mailbox state.
 * - Servo motor for automated opening and closing of the mailbox.
 * - OLED display for real-time status updates.
 * 
 * The system supports saving and loading device credentials using non-volatile storage (Preferences),
 * and provides a web interface for configuration. It also includes mechanisms for message acknowledgment
 * and retransmission in case of communication failures.
 * 
 * @note
 * - Ensure proper wiring of all components before deploying the firmware.
 * - The firmware is designed for IoTBoard hardware and may require modifications for other platforms.
 * @version 3.0
 * @date 2025-03-06
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <iot_board.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <WiFiClientSecure.h>
#include <index_html.h>

#define TRIG D0
#define ECHO D1
#define ROTARY_CLK D6  // Pin del rotary encoder (Clock)
#define ROTARY_DT D8   // Pin del rotary encoder (Data)

#define SERVO_PIN A6 // Pin di controllo del servo

//server object port 80 
AsyncWebServer server(80);
void configureWiFi(wifi_mode_t mode);
// Static IP address
IPAddress local_IP(192, 168, 4, 1);  // IP address
IPAddress gateway(192, 168, 4, 1);   // Gateway
IPAddress subnet(255, 255, 255, 0);  // Subnet Mask
DNSServer dnsServer;
const byte DNS_PORT = 53;
const char *ssidAP = "CtrlMailBoxAP";
const char *passwordAP = "onGso4vnfwTAS42wvs"; // random

Preferences preferences;

String CTRLMAILBOX_KEY = "VSJ5KNVS903N";
String CTRLMAILBOX_NAME = ""; // CMBX1
String MAILTON_KEY = ""; // 00BKFWR39FN4

bool loraFlagReceived = false;
int count = 0;
int count_sent = 0;
bool displayNeedUpdate = false;

// for LoRa communication
uint16_t localAddress = 0x01; 
uint16_t mtAddress = 0;       
int theshold = 2;
bool mail_detected = false;
int readings_made = 0;
String last_lora_msg = "New Mail";
String lora_msg = "";
bool lora_priority = false;
String last_message_received;

// variables for ultrasound sensor
long signal_duration;
int initial_distance = 0;
double distance = 0;
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
const unsigned long waitTime = 20000; // Waiting time for the answer (20 seconds)
bool waitingForResponse = false;

void resetDevice() {
    preferences.begin("device", false);
    preferences.clear();  
    preferences.end();

    display->println("Total reset: all configurations have been deleted!");
    display->display();
    
    // restart the device
    ESP.restart();
}
    
bool saveDeviceCredentials (const String &newMailTon_key, const String &newCtrlMailBox_name, uint16_t &newMailTon_address) {
    preferences.begin("device", false);
    preferences.putString("MailTon_key", newMailTon_key);
    preferences.putString("ctrlmailboxname", newCtrlMailBox_name);
    preferences.putUShort("MailTon_address", newMailTon_address);

    preferences.end();

    display->println("Saved Account credentials:");
    display->println("CtrlMailBox name: " + newCtrlMailBox_name);
    display->println("MailTon key: " + newMailTon_key);
    display->println("MailTon address: " + newMailTon_address);
    display->display();
    return true;
}

// load MailTon/CtrlMailBox data with Preferences
bool loadDeviceCredentials() {
    preferences.begin("device", true);
    MAILTON_KEY = preferences.getString("MailTon_key", "");
    CTRLMAILBOX_NAME = preferences.getString("ctrlmailboxname", "");
    mtAddress = preferences.getUShort("MailTon_address", 0);
    preferences.end();

    if (MAILTON_KEY.isEmpty() || CTRLMAILBOX_NAME.isEmpty() || mtAddress == 0) {
        Serial.println("Account credentials don't find =^.^=");
        return false;
    }

    display->println("Account credentials loaded:");
    display->println("MailTon key: " + MAILTON_KEY);
    display->println("CtrlMailBox name: " + CTRLMAILBOX_NAME); 
    display->println("Mailton address: " + mtAddress); 
    display->display();
    return true;
}

// Callback for the main page
void handleRoot(AsyncWebServerRequest *request) {
    request->send(200, "text/html", index_html);
}

// Callback to manage the form
void handleSave(AsyncWebServerRequest *request) {
    if(request->hasParam("ctrlmailboxname", true) && request->hasParam("mailtonkey", true) && request->hasParam("mailtonaddress", true)) {
        CTRLMAILBOX_NAME = request->getParam("ctrlmailboxname", true)->value();
        MAILTON_KEY = request->getParam("mailtonkey", true)->value();
        String inputAddress = request->getParam("mailtonaddress", true)->value();
        
        if (inputAddress.startsWith("0x") || inputAddress.startsWith("0X")) {
            mtAddress = strtol(inputAddress.c_str(), NULL, 16);
        } else {
            mtAddress = inputAddress.toInt();
        }

        // use preferences to save
        if (saveDeviceCredentials(MAILTON_KEY, CTRLMAILBOX_NAME, mtAddress)) {
            request->send(200, "text", "document.getElementById('savedCredentials').style.display='block';");
        } else {
            request->send(500, "text", "document.getElementById('errorCredentials').style.display='block';");
        }        
        
        // after saving the credentials, changes to ap_sta modes
        display->println("Saved Device data");
        display->println("CtrlMailBox name: " + CTRLMAILBOX_NAME); 
        display->println("MailTon key: " + MAILTON_KEY);
        display->println("MailTon address: " + inputAddress); 
        display->display();
        delay(2000);

    } else {
        request->send(500, "text", "document.getElementById('errorCredentials').style.display='block';");
    }
}  

void configureWiFi() {
    // configure the form as an access point
    WiFi.softAPConfig(local_IP, gateway, subnet);

    WiFi.softAP(ssidAP, passwordAP);
    
    // the DNS Server redirects all requests to WiFi.softAPIP() and activate router management 
    dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

    // HTTP | configure the server routes
    server.on("/", HTTP_GET, handleRoot);
    server.on("/", HTTP_POST, handleSave);
    server.begin();
}

void IRAM_ATTR rotaryChanged() {
    wait_rotary = true;
    if (digitalRead(ROTARY_CLK) == digitalRead(ROTARY_DT)) {
        mailbox_open = false;
    } else {
        mailbox_open = true;
    }
    wait_rotary = false;
}

// function to generate the PWM signal for the servant
void writeServo(int angle) {
    wait_servo = true;
    // per mappare l'angolo in un intervallo di larghezza dell'impulso
    int pulseWidth = map(angle, 0, 180, 1000, 2000); 
    

    /**
     * @brief  PWM signal 
     * 
     * @param pulseWidth variable width impulse/ impulso di larghezza variabile
     */
    digitalWrite(SERVO_PIN, HIGH);
    delayMicroseconds(pulseWidth);  
    digitalWrite(SERVO_PIN, LOW);
    delay(20 - pulseWidth / 1000);  // PWM  cycle (20 ms)

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

    // Estrai nome, key e dati
    String receiverName = extractValue(incoming, "NAME");
    String receiverKey = extractValue(incoming, "CTRLMAILBOX_KEY");
    String senderKey = extractValue(incoming, "MAILTON_KEY");
    String data = extractValue(incoming, "DATA");

    if (receiverName != CTRLMAILBOX_NAME ||  receiverKey != CTRLMAILBOX_KEY || senderKey != MAILTON_KEY) {
        display->println("Address or key mismatch detected:");
        display->println("Recipient: " + recipientStr + ", Local: " + localAddressStr);
        display->println("Sender: " + senderStr + ", CMB: " + ctrlmbAddressStr);
        display->println("Receiver Name: " + receiverName + ", Expected: " + CTRLMAILBOX_NAME);
        display->println("Sender Key: " + senderKey + ", Expected: " + MAILTON_KEY);
        display->println("Receiver Key: " + receiverKey + ", Expected: " + CTRLMAILBOX_KEY);
        //display->display();
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
    display->printf("Message: %s\n", data);
    display->printf("RSSI: %d\n", lora->packetRssi());
    display->printf("Snr: %02f\n", lora->packetSnr());
    
    lora->receive();
    last_message_received = data;
    loraFlagReceived = true; // set the reception flag
}

void onLoRaSend() {
}

void sendMessageLoRa() {
    lora_priority = true;
    digitalWrite(LED_GREEN, HIGH);
    last_lora_msg = lora_msg; // modified at each call
    
    String messageToSend = "NAME=" + CTRLMAILBOX_NAME + ";CTRLMAILBOX_KEY=" + CTRLMAILBOX_KEY + ";MAILTON_KEY=" + MAILTON_KEY + ";DATA=" + lora_msg;

    lora->beginPacket();

    lora->write(mtAddress);                // add mtAddress address
    lora->write(localAddress);             // add sender address
    lora->write(count_sent);               // add message ID
    lora->write(messageToSend.length());   // add payload length

    // XOR 
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
    waitingForResponse = true; // set the waiting flag
    lastSendTime = millis(); // record the sending time

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
    
    double signal_duration = pulseIn(ECHO, HIGH, 30000);
    double cm = signal_duration / 58;
    // for the range
    if(cm > 1000) cm = 1;
    return (cm > 0 && cm < 100) ? cm : 100;
}

int measuresAverageDistance() {
    long sum = 0;
    int readings = 25;
    for (int i = 0; i < readings; i++) {
        double distance = measuresDistance();
        sum += distance;
        delay(15);
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
    loadDeviceCredentials();

    configureWiFi();

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
    dnsServer.processNextRequest();
    
    delay(1);
    
    if(displayNeedUpdate){
        displayNeedUpdate = false;
        display->display();
    }
    

    if (!lora_priority && !wait_rotary && !wait_servo && !mailbox_open) {
        // update the initial distance periodically
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
        // send the message only if the mailbox is open and the mail is taken (or the distance is the initial one)
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

    // timer to automatically close the mailbox if left open for too long
    static unsigned long mailboxOpenTime = 0;
    const unsigned long maxOpenDuration = 60000; // 60 seconds

    if (mailbox_open) {
        if (mailboxOpenTime == 0) {
            mailboxOpenTime = millis(); // Start the timer when the mailbox is opened
        } else if (millis() - mailboxOpenTime > maxOpenDuration) {
            mailbox_open = false; // Close the mailbox
            writeServo(90); // Move servo to close position
            mailboxOpenTime = 0; // Reset the timer
            display->println("Mailbox auto-closed due to timeout.");
            display->display();
        }
    } else {
        mailboxOpenTime = 0; // Reset the timer if the mailbox is closed
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
    
    //delay(1000);
    
    if (!mailbox_open){
        writeServo(90);
        delay(10);
    }else {
        writeServo(-30); 
        delay(10);
    }


    display->clearDisplay();
    display->setCursor(0,0);
    display->printf("Distance: %.2f cm\n", distance);
    display->println(mailbox_open ? "MAILBOX OPEN =^.^= " : "MAILBOX CLOSE");
    display->printf("CMB addr: %04X\n", localAddress);
    display->println("CMB Name: " + CTRLMAILBOX_NAME);
    display->println("CMB KEY: " + CTRLMAILBOX_KEY);
    display->printf("MT addr:  %04X\n",  mtAddress);
    display->println("MT KEY: " + MAILTON_KEY );
    display->display();
    delay(1);
}