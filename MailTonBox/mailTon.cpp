/**
 * @file mailTon.cpp
 * @brief This file contains the implementation of the MailTonBox IoT system.
 * 
 * The system is designed to monitor and manage a mailbox using various sensors and communication protocols.
 * It includes functionalities for Wi-Fi configuration, Telegram bot integration, LoRa communication, 
 * and AI-based predictions for environmental conditions.
 * 
 * Key functionalities:
 * - Wi-Fi configuration through a web interface or Telegram bot.
 * - Telegram bot for notifications and remote management.
 * - LoRa communication for receiving messages from remote sensors (ctrlMailBox).
 * - AI-based predictions for temperature, humidity, and air quality.
 * - Web server to serve configuration pages and handle form submissions.
 * 
 * The system uses various libraries including Arduino, WiFi, AsyncTCP, ESPAsyncWebServer, DNSServer, Preferences, 
 * ArduinoJson, WiFiClientSecure, UniversalTelegramBot, Adafruit_Sensor, DHT, and EloquentTinyML.
 * 
 * @version 3.2
 * @date 2025-03-05
 * 
 * @note Ensure to configure the Wi-Fi and Telegram bot credentials before deploying the system.
 * 
 * @note The system receives environmental data from another device (ctrlMailBox) located in the mailbox via LoRa communication.
 * 
 * @copyright Copyright (c) 2025
 */

#include <Arduino.h>
#include <WiFi.h>
#include <iot_board.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <EloquentTinyML.h>
#include "model.h"  // AI model
#include <index_html.h>

#define DHTPIN  D1   
#define DHTTYPE DHT11 
DHT dht(DHTPIN, DHTTYPE);


#define NUMBER_OF_INPUTS 3  // used number of features
#define NUMBER_OF_OUTPUTS 3 // regression output
#define TENSOR_ARENA_SIZE 8*1024  // memory size for Tensorflow Lite

// init object tinyml
Eloquent::TinyML::TfLite<NUMBER_OF_INPUTS, NUMBER_OF_OUTPUTS, TENSOR_ARENA_SIZE> ml;

// MinMaxscaler
// minimum and maximum of the first feature
#define INPUT_MIN_1 -10.0  
#define INPUT_MAX_1 49.0   
// minimum and maximum of the second feature
#define INPUT_MIN_2 0.0  
#define INPUT_MAX_2 100.0 
// minimum and maximum of the third feature
#define INPUT_MIN_3 0.0 
#define INPUT_MAX_3 50.0

float temperature, humidity = 0.0;
float ppm_value = 0.0;
int temp_humidity_class = 0;
int ppm_class = 0;

void DetectionAndPrediction();
String tempHumidityClassToText(int value);
String ppmClassToText(int value);

//server object port 80 
AsyncWebServer server(80);
DNSServer dnsServer;
const byte DNS_PORT = 53;

Preferences preferences;

// WiFi configuration
void configureWiFi(wifi_mode_t mode);
// Static IP address
IPAddress local_IP(192, 168, 4, 1);  // IP address
IPAddress gateway(192, 168, 4, 1);    // Gateway
IPAddress subnet(255, 255, 255, 0);   // Subnet Mask
String ssid;
String password;
bool firstConnection = false;
bool connected = false;
const char *ssidAP = "MailTonAP";
const char *passwordAP = "ka2smfVFka9fSsk3U"; // random 

// Telegram bot
WiFiClientSecure net_ssl;
String username="", user_password = "";
String telegramToken = "";
UniversalTelegramBot bot("7411303563:AAFfc1YBJ1CwwfKZefxLoYi2wxbwMt6Dbg0", net_ssl);
int Bot_mtbs = 200;
long Bot_lasttime;
String chat_id;
String from_name;
bool bot_active =  false;
bool bot_started = false;
String savedChatID = "";  
String savedUsername = "";
bool botConfigureAccount = false;
bool userFlagBot, passFlagBot = false;
bool botConfigureWiFi = false;
bool newWIFIbyBot = false;
bool newPASSbyBot = false;
bool allertAI = true;

// LoRa variables
int count = 0;
int count_sent = 0;
bool loraFlagReceived = false;
bool loraFlagError = false;
uint16_t localAddress = 0x02;     
uint16_t ctrlmbAddress = 0;
String lora_msg = ""; // payload of packet
bool lora_priority = false;

const String MAILTON_KEY =  "00BKFWR39FN4"; // random

String CTRLMAILBOX_NAME = ""; // CMBX1
String CTRLMAILBOX_KEY = ""; // VSJ5KNVS903N
bool deviceInfo = false;

void resetDevice() {
    // opens all preferences to delete them
    preferences.begin("wifi", false);
    preferences.clear(); 
    preferences.end();
    
    preferences.begin("telegram", false);
    preferences.clear();  
    preferences.end();
    
    preferences.begin("lora", false);
    preferences.clear();  
    preferences.end();

    preferences.begin("account", false);
    preferences.clear();  
    preferences.end();

    preferences.begin("ctrlmailbox", false);
    preferences.clear();
    preferences.end();

    display->println("Total reset: all configurations have been deleted!");
    display->display();
    
    // restart the device
    ESP.restart();
}
    
bool saveCtrlMailBoxCredentials (const String &newCtrlMailBox_key, const String &newCtrlMailBox_name,const uint16_t &newCtrlMailBox_address) {
    preferences.begin("ctrlmailbox", false);
    preferences.putString("ctrlmailboxkey", newCtrlMailBox_key);
    preferences.putString("ctrlmailboxname", newCtrlMailBox_name);
    preferences.putUShort("ctrlmailboxaddr", newCtrlMailBox_address);
    preferences.end();

    display->println("Saved CtrlMailBox credentials:");
    display->println("CtrlMailBox key: " + newCtrlMailBox_key);
    display->println("CtrlMailBox name: " + newCtrlMailBox_name);
    display->println("CtrlMailBox address: " + newCtrlMailBox_address);
    display->display();
    return true;
}

// load CtrlMailBox data with Preferences
bool loadCtrlMailBoxCredentials() {
    preferences.begin("ctrlmailbox", true);
    CTRLMAILBOX_KEY = preferences.getString("ctrlmailboxkey", "");
    CTRLMAILBOX_NAME = preferences.getString("ctrlmailboxname", "");
    ctrlmbAddress = preferences.getUShort("ctrlmailboxaddr", 0);
    preferences.end();

    if (CTRLMAILBOX_KEY.isEmpty() || CTRLMAILBOX_NAME.isEmpty() || ctrlmbAddress == 0) {
        Serial.println("CtrlMailBox credentials don't find ");
        return false;
    }
    
    display->clearDisplay();
    display->setCursor(0,0);
    display->printf("MT addr:  %04X\n", localAddress);
    display->println("MT KEY: " + MAILTON_KEY );
    // cred load 
    display->printf("CMB addr: %04X\n",ctrlmbAddress);
    display->println("CMB Name: " + CTRLMAILBOX_NAME);
    display->println("CMB KEY: " + CTRLMAILBOX_KEY);
    display->display();
    return true;
}

// save Account credentials with Preferences
bool saveAccountCredentials(const String &newUsername, const String &newUser_password, const String &newTelegramToken) {
    preferences.begin("account", false);
    preferences.putString("username", newUsername);
    preferences.putString("user_password", newUser_password);
    preferences.putString("Telegram Token", newTelegramToken);
    preferences.end();

    display->println("Saved Account credentials:");
    display->println("Username: " + newUsername);
    display->println("Password: " + newUser_password);
    display->println("Telegram Token: " + newTelegramToken); 
    display->display();
    return true;
}

// load Account credentials with Preferences
bool loadAccountCredentials() {
    preferences.begin("account", true);
    username = preferences.getString("username", "");
    user_password = preferences.getString("user_password", "");
    telegramToken = preferences.getString("telegramToken", "");
    preferences.end();

    if (username.isEmpty() || user_password.isEmpty()) {
        Serial.println("Account credentials don't find");
        return false;
    }

    display->println("Account credentials loaded:");
    display->println("Username: " + username);
    display->println("Password: " + user_password); 
    display->println("Telegram Token: " + telegramToken); 
    display->display();
    return true;
}

// save the details of the chat for the Telegram Bot
bool saveChatDetails(const String &newChatID, const String &newUsername) {
    savedChatID = newChatID;
    savedUsername = newUsername;
    Preferences preferences;
    // Namespace "telegram" to save the chat_id and username
    preferences.begin("telegram", false); // false -> for read/write mode
    preferences.putString("chat_id", newChatID);
    preferences.putString("username", newUsername);
    preferences.end();

    display->println("Chat ID salvato:");
    display->println(newChatID);
    display->println("Username salvato:");
    display->println(newUsername);
    display->display();
    return true;
}

// load the details of the chat for the Telegram Bot
bool loadChatDetails() {
    Preferences preferences;
    preferences.begin("telegram", true);  // true -> reading only
    savedChatID = preferences.getString("chat_id", "");
    savedUsername = preferences.getString("username", "");
    preferences.end();

    // chatID not found
    if (savedChatID.isEmpty()) 
        return false;
    
    display->println("Chat ID loaded:");
    display->println(savedChatID);
    display->println("Username loaded:");
    display->println(savedUsername);
    display->display();
    return true;
}


// save WiFi credentials with Preferences
bool saveWiFiCredentials(const String &newSSID, const String &newPassword) {
    preferences.begin("wifi", false);
    preferences.putString("ssid", newSSID);
    preferences.putString("password", newPassword);
    preferences.end();

    display->println("Saved WiFi credentials:");
    display->println("SSID: " + newSSID);
    display->println("Password: " + newPassword);
    display->display();
    return true;
}

// load credentials with Preferences
bool loadWiFiCredentials() {
    preferences.begin("wifi", true);
    ssid = preferences.getString("ssid", "");
    password = preferences.getString("password", "");
    preferences.end();

    if (ssid.isEmpty() || password.isEmpty()) {
        Serial.println("WiFi credentials you don't find");
        return false;
    }

    display->println("Loaded WiFi credentials:");
    display->println("SSID: " + ssid);
    display->println("Password: " + password); 
    display->display();
    return true;
}

// Callback for the main page
void handleRoot(AsyncWebServerRequest *request) {
    request->send(200, "text/html", index_html);
    //request->send(SPIFFS, "/index.html", "text/html");
}

// Callback for the tset route
void handleManifestRoute(AsyncWebServerRequest *request) {
    request->send(200, "application/json", manifest_json);
}

// Callback to manage the form
void handleSave(AsyncWebServerRequest *request) {
    if(request->hasParam("ssid", true) && request->hasParam("password", true)) {
        ssid = request->getParam("ssid", true)->value();
        password = request->getParam("password", true)->value();
        
        // use preferences to save
        if (saveWiFiCredentials(ssid, password)) {
            request->send(200, "text", "document.getElementById('savedCredentials').style.display='block';");
        } else {
            request->send(500, "text", "document.getElementById('errorCredentials').style.display='block';");
        }        
        
        // after saving the credentials, changes to ap_sta modes
        display->println("Saved WiFi data");
        display->println("SSID: " + ssid);
        display->println("Password: " + password);
        display->display();
        delay(2000);
        configureWiFi(WIFI_AP_STA);

    } else if(request->hasParam("username", true) && request->hasParam("user_password", true) && request->hasParam("token", true)) {
        username = request->getParam("username", true)->value();
        user_password = request->getParam("user_password", true)->value();
        telegramToken = request->getParam("token", true)->value();
        
        // use preferences to save
        if (saveAccountCredentials(username, user_password, telegramToken)) {
            request->send(200, "text", "document.getElementById('savedCredentials').style.display='block';");
        } else {
            request->send(500, "text", "document.getElementById('errorCredentials').style.display='block';");
        }        
        
        // after saving the credentials, changes to ap_sta modes
        display->println("Saved MailTon data");
        display->println("Username: " + username);
        display->println("Password: " + user_password); 
        display->println("Telegram Token: " + telegramToken); 
        display->display();
        delay(2000);
        
    } else if(request->hasParam("ctrlmailboxkey", true) && request->hasParam("ctrlmailboxname", true)  && request->hasParam("ctrlmailboxaddress", true)) {
        CTRLMAILBOX_KEY = request->getParam("ctrlmailboxkey", true)->value();
        CTRLMAILBOX_NAME = request->getParam("ctrlmailboxname", true)->value();
        String inputAddress = request->getParam("ctrlmailboxaddress", true)->value();
        
        if (inputAddress.startsWith("0x") || inputAddress.startsWith("0X")) {
            ctrlmbAddress = strtol(inputAddress.c_str(), NULL, 16);
        } else {
            ctrlmbAddress = inputAddress.toInt();
        }

        // use preferences to save
        if (saveCtrlMailBoxCredentials(CTRLMAILBOX_KEY, CTRLMAILBOX_NAME, ctrlmbAddress)) {
            request->send(200, "text", "document.getElementById('savedCredentials').style.display='block';");
        } else {
            request->send(500, "text", "document.getElementById('errorCredentials').style.display='block';");
        }        
        
        // after saving the credentials, changes to ap_sta modes
        display->println("Saved CtrlMailBox data");
        display->println("CtrlMailBox key: " + CTRLMAILBOX_KEY);
        display->println("CtrlMailBox name: " + CTRLMAILBOX_NAME); 
        display->println("CtrlMailBox address: " + inputAddress); 
        display->display();
        delay(2000);

    } else {
        request->send(500, "text", "document.getElementById('errorCredentials').style.display='block';");
    }
}  


void on_connected_station(WiFiEvent_t event, WiFiEventInfo_t info) {
    display->println("ConnectionAP");
    display->print("Mac address of the client: ");
    display->printf(MACSTR "\n", MAC2STR(info.wifi_ap_staconnected.mac));
    display->println(IPAddress(info.wifi_ap_staconnected.aid));
    display->display();
}

// configure the wifi depending on the way
void configureWiFi(wifi_mode_t mode) {
    switch (mode) {
        case WIFI_AP:
            WiFi.softAPConfig(local_IP, gateway, subnet);
            // configure the form as an access point
            WiFi.softAP(ssidAP, passwordAP);
            //WiFi.onEvent(on_connected_station, ARDUINO_EVENT_WIFI_AP_STACONNECTED);
            
            // the DNS Server redirects all requests to WiFi.softAPIP() and activate router management 
            dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

            // HTTP | configure the server routes
            server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
                request->send(200, "image/x-icon", ""); // puoi anche mandare un'icona vuota
              });
              
            server.on("/", HTTP_GET, handleRoot);
            server.on("/manifest.json", HTTP_GET, handleManifestRoute);
            server.on("/", HTTP_POST, handleSave);
            server.begin();
            
            break;
        case WIFI_AP_STA:
            // configuration AP mode
            WiFi.disconnect(true);
            WiFi.mode(WIFI_AP_STA);
            WiFi.softAPConfig(local_IP, gateway, subnet);
            WiFi.softAP(ssidAP, passwordAP);

            dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
            server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
                request->send(200, "image/x-icon", ""); // puoi anche mandare un'icona vuota
              });              
            server.on("/", HTTP_GET, handleRoot);
            server.on("/", HTTP_POST, handleSave);
            server.on("/manifest.json", HTTP_GET, handleManifestRoute);
            server.begin();

            // configure Station mode
            WiFi.begin(ssid, password);
            
            display->clearDisplay();
            display->setCursor(0,0);
            display->println("Connecting to WiFi...");
            display->display();
            
            unsigned long startAttemptTime = millis();
            const unsigned long timeout = 5000; // 5s of timeout
            
            while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < timeout) {
                delay(1000);
                display->print(".");
                display->display();
            }
            
            if(WiFi.status() == WL_CONNECTED) {
                firstConnection = true;
                display->clearDisplay();
                display->setCursor(0,0);
                display->println("Connected to WiFi");                
                display->println("SSID: " + ssid);
                display->display();
            } else {
                display->clearDisplay();
                display->setCursor(0,0);
                display->println("Connection failed, switching back to AP mode");
                display->display();
                configureWiFi(WIFI_AP);
            }
            break;
    }
}

/**
 * @brief manages the messages received from a telegram bot (TonyBot).
 * Depending on the command received, 
 * (/start, /restart, /description, /about, /commands, /configurewifi, /ai, /allert_ai), 
 * performs different actions such as saving chat details, send reply messages,
 * configure wifi, or perform forecasts with the AI. =^.^=
 * 
 * @param m the message from the Telegram bot, 
 * of type `telegramMessage` struct, containing the message details
 */
void handleNewMessages(telegramMessage m) {
    bot_active = true;
    chat_id = m.chat_id;
    from_name = m.from_name;
    String text = m.text;

    if (from_name == "") from_name = "Guest";

    display->clearDisplay();
    display->setCursor(0, 0);
    display->printf("%s:\n", from_name.c_str());
    display->printf("%s\n", text.c_str());
    display->display();

    if(!bot_started){
        if (text == "/start") {
            if(loadAccountCredentials()){
                bot.sendMessage(chat_id, "Hello " + from_name + "! I am TonyBot 🤖 a bot telegram on Mailton 📬🔔 (Ardunio)! \n\n When the mail arrives, PostaLino📪📣 will notify us. \nSee you when the mail arrives! 👀📨");
                bot.sendMessage(chat_id, "🆗 Send me the Username of your Account !", "");
                botConfigureAccount = true;
                userFlagBot = true;
                return;
            } else{
                return;
            }
        } 
    }

    if (savedChatID != chat_id && !botConfigureAccount)
        return;

    if (text == "/reboot") {
        bot.sendMessage(chat_id, "Reboot in progress on Mailton 📬🔔! \nBye bye " + from_name + "👋.");
        botConfigureWiFi = false;
        resetDevice();
    } else if (text == "/restart") {
        bot.sendMessage(chat_id, "Hello  " + from_name + "!🔔\n MailTon 📬🔔 has been restarted, but now I'm active again!🚀");
        botConfigureWiFi = false;
    } else if (text == "/description") {
        bot.sendMessage(chat_id, "🚀");
        botConfigureWiFi = false;
    } else if (text == "/visibility") {
        if (bot_started) bot.sendMessage(chat_id, "🆗 Send the /start command with the other device you want to add! Mailton is available for new chats!🚀");
        else bot.sendMessage(chat_id, "🆗 Mailton is hidden, is not available for new chats!🚀");
        bot_started = !bot_started;
        botConfigureWiFi = false;
        botConfigureAccount = false;
    } else if (text == "/commands") {
        bot.sendMessage(chat_id, "🎛️  Available commands:\n\n     ✅  /start - Start the bot\n\n     🔄️  /restart - Restart the bot\n\n     📑  /description - Get a description\n\n     📄  /about - About this bot\n\n     🕹️  /commands - List all commands \n\n     🛜  /configurewifi - Configure your WiFi \n\n     ❇️  /ai - Artificial Intelligence \n\n     ⚠️  /allert_ai - On/Off Alert notification from AI \n\n");
        botConfigureWiFi = false;
    } else if (text == "/configurewifi") {
        bot.sendMessage(chat_id, "🆗 Send me the new SSDI of your WiFi 🛜", "");
        botConfigureWiFi = true;
        newWIFIbyBot = true;
        return;
    } else if(text == "/ai"){
        DetectionAndPrediction();
        String message = "🏠 Here are values ​​in your room in real time!  \n\n";
        message += "    📟  Real real values ​​from Mailton: \n\n";
        message += "        🌡️  Temperature: " + String(temperature) + " °C\n\n";
        message += "        💧  Moisture: " + String(humidity) + " %\n\n";
        message += "    🤖  Values ​​predicted by the AI: \n\n";
        message += "        🔮  PPM CO predicted: " + String(ppm_value) + "\n\n";
        message += "        📋  Temperature & Humidity Class: \n";
        message += "               " + String(temp_humidity_class) + " = " + tempHumidityClassToText(temp_humidity_class) + "\n\n";
        message += "        🛡️  PPM CO Class: " + String(ppm_class) + " = " + ppmClassToText(ppm_class) + "\n\n";
        bot.sendMessage(chat_id, message);
        botConfigureWiFi = false;
    }else if(text == "/allert_ai"){
        allertAI = !allertAI;
        if (allertAI) bot.sendMessage(chat_id, "🔔 L'allert of forecasts with AI is active! 🤖", "");
        else bot.sendMessage(chat_id, "🔕 OK. L'allert of forecasts with AI is disabled. 🛑", "");
        botConfigureWiFi = false;
    }else if(!botConfigureWiFi && !botConfigureAccount && bot_started){
        bot.sendMessage(chat_id, "Unknown command. Type /commands to see the list of available commands.");
    }
    
    if(botConfigureAccount){
        if (userFlagBot){ 
            if(username == text){
                bot.sendMessage(chat_id, "🆗 Now send me the password🔑", "");
                userFlagBot = false;
                passFlagBot = true;
                return;
            }else{
                bot.sendMessage(chat_id, "❌ Wrong username, try again!", "");
                return;
            }
        } 
        if (passFlagBot){
            if (user_password == text){
                bot.sendMessage(chat_id, "🆗 Saved credentials🔑!.", "");
                bot.sendMessage(chat_id, "✅ Access successfully made!", "");
                passFlagBot = false;
                
                saveChatDetails(chat_id, from_name);
                botConfigureAccount = false;
                bot_started = true;
                return;
            }else{
                bot.sendMessage(chat_id, "❌ Wrong password, try again!", "");
                return;
            }
        }  
    }

    // for wifi configuration
    if (botConfigureWiFi){
        if (newWIFIbyBot){
            ssid = text;
            bot.sendMessage(chat_id, "🆗 Now send me the password🔑 of your WiFi 🛜", "");
            newWIFIbyBot = false;
            newPASSbyBot = true;
            return;
        } 
        if (newPASSbyBot){
            bot.sendMessage(chat_id, "🆗 Saved credentials🔑! Now wait for the connection restart 🔄️.", "");
            bot.sendMessage(chat_id, "⏱️", "");
            newPASSbyBot = false;
            password = text;
            saveWiFiCredentials(ssid, password);
            WiFi.disconnect(true);
            WiFi.mode(WIFI_AP_STA);
            botConfigureWiFi = false;
            return;
        }            
    }
}

/**
 * @brief Manager to send messages to the bot 
 * in relation to the messages received with LoRa
 */
void handlerSendMessage() {
    display->clearDisplay();
    display->setCursor(0,0);
    
    if(bot_active){
        if (lora_msg == "Mailbox Opened") {
            bot.sendMessage(chat_id, "📬 Mailbox Opened, someone is already withdrawing the mail 📭");
            digitalWrite(LED_YELLOW, LOW);
            display->println("Send to bot: Mailbox Opened");
            display->display();
        }else if (lora_msg == "New Mail"){
            static bool mFlag= false;
            if(mFlag) bot.sendMessage(chat_id, "🔔 Lino the Postino warned me that there is placed in the mailbox! 📬");
            else bot.sendMessage(chat_id, "🔔 PostaLino warned me that there is placed in the mailbox! 📬");
            mFlag = !mFlag;
            digitalWrite(LED_YELLOW, HIGH);
            display->println("Send to bot: mail in the mailbox!");
            display->display();
        } else {
            bot.sendMessage(chat_id, "❌ Error test! 📨");
            display->println("Send to bot: Error test!");
            loraFlagError = true;
            digitalWrite(LED_RED, HIGH);
        }
    }else{
        display->println("Bot not activated, not sent notification");
        display->display();
    }
}

// Restart message, to reactivate the sending of messages to the bot
void SendRestartMessageBot(){
    telegramMessage restartMsg;
    restartMsg.chat_id = savedChatID;
    restartMsg.text = "/restart";         
    restartMsg.from_name = savedUsername;   
    // call the HandleNewMessages() by passing the restart message
    handleNewMessages(restartMsg);
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
    lora_priority = true;
    if (packetSize == 0) return;

    // read packet header bytes:
    uint16_t recipient = lora->read();  // recipient address
    uint16_t sender = lora->read();     // sender address
    byte incomingMsgId = lora->read();  // incoming msg ID
    byte incomingLength = lora->read(); // incoming msg length
    byte receivedChecksum = lora->read(); // read checksum

    String incoming = ""; // payload of packet

    while (lora->available()) {          // can't use readString() in callback, so
        incoming += (char)lora->read(); // add bytes one by one
    }

    byte calculatedChecksum = 0;
    for (int i = 0; i < incoming.length(); i++) {
        calculatedChecksum ^= incoming[i];
    }

    if (incomingLength != incoming.length() || receivedChecksum != calculatedChecksum) { 
        display->clearDisplay();
        display->setCursor(0,0);
        display->println("Error: checksum mismatch");
        loraFlagError = true;
        digitalWrite(LED_RED, HIGH);
        return;
    }

    // Extract names, key and data
    String senderName = extractValue(incoming, "NAME");
    String senderKey = extractValue(incoming, "CTRLMAILBOX_KEY");
    String receiverKey = extractValue(incoming, "MAILTON_KEY");
    String data = extractValue(incoming, "DATA");

    if (senderName != CTRLMAILBOX_NAME || senderKey != CTRLMAILBOX_KEY || receiverKey != MAILTON_KEY) {
        //display->println("Address or key mismatch detected:");
        //display->println("Recipient: " + recipientStr + ", Local: " + localAddressStr);
        //display->println("Sender: " + senderStr + ", CMB: " + ctrlmbAddressStr);
        display->println("Sender Name: " + senderName + ", Expected: " + CTRLMAILBOX_NAME);
        display->println("Sender Key: " + senderKey + ", Expected: " + CTRLMAILBOX_KEY);
        display->println("Receiver Key: " + receiverKey + ", Expected: " + MAILTON_KEY);
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
    display->printf("Name: %s\n", senderName.c_str());
    display->printf("Message: %s\n", data.c_str());
    display->printf("RSSI: %d\n", lora->packetRssi());
    display->printf("Snr: %02f\n", lora->packetSnr());
    
    lora->receive();

    if(!loraFlagError){    
        lora_msg = data.c_str();
    }

    loraFlagReceived = true;
    lora_priority = false;
}

// this function uses the interrupt when it is called
void onLoRaSend(){    
}

// send message LoRa (without interrupt)
void sendMessageLoRa(const String &loraSendMsg) {
    digitalWrite(LED_RED, HIGH);
      
    //Preferences preferences;
    //preferences.begin("lora", true);
    //String senderName = CTRLMAILBOX_NAME;
    //String senderKey = MAILTON_KEY;
    //String receiverKey = CTRLMAILBOX_KEY;

    //String senderKey = preferences.getString("MAILTON_KEY", "");  
    //preferences.end();

    // Componi il messaggio nel formato: NAME=XYZ;KEY=ABC;DATA=Messaggio
    String messageToSend = "NAME=" + CTRLMAILBOX_NAME + ";CTRLMAILBOX_KEY=" + CTRLMAILBOX_KEY + ";MAILTON_KEY=" + MAILTON_KEY + ";DATA=" + loraSendMsg;

    lora->beginPacket();

    lora->write(ctrlmbAddress);              // add ctrlmbAddress address
    lora->write(localAddress);             // add sender address
    lora->write(count_sent);               // add message ID
    lora->write(messageToSend.length());   // add payload length

    byte checksum = 0;
    for (int i = 0; i < messageToSend.length(); i++) {
        checksum ^= messageToSend[i];
    }
    lora->write(checksum); // add checksum
    lora->print(messageToSend); // add payload

    lora->endPacket(true); // true=>async, non-blocking mode
    count_sent++;
    lora->receive();

    delay(100);
    loraFlagError = false;

    digitalWrite(LED_RED, LOW);
}

//? ************ debug for AI model *************
void printArray(float* arr, int size) {
    String message;
    // print every element of array
    for (int i = 0; i < size; i++) {
        message += "y_pred[" + String(i) + "] = " + String(arr[i]) + "=^.^=\n";
    }
    bot.sendMessage(chat_id, message);
}
void printClassAndProbability(float* arr, int size) {
    int index = 0;
    float maxVal = arr[0];
    // find the class index with maximum probability =^.^=
    for (int i = 1; i < size; i++) {
        if (arr[i] > maxVal) {
            maxVal = arr[i];
            index = i;
        }
    }
}
//? *******************************************

// classes conversion
String tempHumidityClassToText(int value) {
    switch(value) {
        case 0: return "Low Temperature, Dry Humidity";
        case 1: return "Low Temperature, Normal Humidity";
        case 2: return "Low Temperature, Humid";
        case 3: return "Medium Temperature, Dry Humidity";
        case 4: return "Medium Temperature, Normal Humidity";
        case 5: return "Medium Temperature, Humid";
        case 6: return "High Temperature, Dry Humidity";
        case 7: return "High Temperature, Normal Humidity";
        case 8: return "High Temperature, Humid";
        default: return "Unknown Class";
    }
}

String ppmClassToText(int value) {
    switch(value) {
        case 0: return "Good Air Quality";
        case 1: return "Moderate Air Quality";
        case 2: return "Poor Air Quality";
        case 3: return "Very Poor Air Quality";
        default: return "Unknown Class";
    }
}

// search and return the index that contains the highest probability
int maxIndex(float* arr, int size) {
        int index = 0;
        float maxVal = arr[0];
        for (int i = 1; i < size; i++) {
            if (arr[i] > maxVal) {
                maxVal = arr[i];
                index = i;
            }
        }
        return index;
}

void SendAllertMsgBot(){
    if (bot_active && allertAI) {
            String message = "⚠️ Attention these are the values ​​in your room in real time!\n\n";
            message += "    📟  Real real values ​​from Mailton: \n\n";
            message += "        🌡️  Temperature: " + String(temperature) + " °C\n\n";
            message += "        💧  Moisture: " + String(humidity) + " %\n\n";
            message += "    🤖  Values ​​predicted by the AI: \n\n";
            message += "        🔮  PPM CO predicted: " + String(ppm_value) + "\n\n";
            message += "        📋  Temperature & Humidity Class: \n";
            message += "               " + String(temp_humidity_class) + " = " + tempHumidityClassToText(temp_humidity_class) + "\n\n";
            message += "        🛡️  PPM CO Class: " + String(ppm_class) + " = " + ppmClassToText(ppm_class) + "\n\n";
            bot.sendMessage(chat_id, message);
    }   
}

// measurements and predictions of model values ​​ai
void DetectionAndPrediction(){

    // Detection with dht11 sensor
    //display->clearDisplay();
    display->clearDisplay();
    display->setCursor(0,16);
    float newT = dht.readTemperature();

    if (isnan(newT)) {
        display->println(F("X DHT ERROR"));
        display->display();
    } else {
    temperature = newT;
    display->print("TEMPERATURE = ");
    display->println(temperature);
    display->display();
    }

    float newH = dht.readHumidity();

    if (isnan(newH)) {
    display->println(F("X DHT ERROR"));
    display->display();
    } else {
    humidity = newH;
    display->print("HUMIDITY = ");
    display->println(humidity);
    display->display();

    }

    // -------------------- PREDICTION AND CLASSIFICATION -----------------------

    float raw_input[NUMBER_OF_INPUTS] = {temperature, humidity, ppm_value}; 

    // MinMaxScaler normalization
    float input[NUMBER_OF_INPUTS];
    input[0] = (raw_input[0] - INPUT_MIN_1) / (INPUT_MAX_1 - INPUT_MIN_1);
    input[1] = (raw_input[1] - INPUT_MIN_2) / (INPUT_MAX_2 - INPUT_MIN_2);
    input[2] = (raw_input[2] - INPUT_MIN_3) / (INPUT_MAX_3 - INPUT_MIN_3);

    // Inference
    float y_pred[NUMBER_OF_OUTPUTS];
    ml.predict(input, y_pred);

    // the aforementioned value of PPM decodes (if it has been climbed between 0-1)
    ppm_value = y_pred[0] * (INPUT_MAX_3 - INPUT_MIN_3) + INPUT_MIN_3;
    
    temp_humidity_class = y_pred[1];
    ppm_class = y_pred[2];

    temp_humidity_class = maxIndex(y_pred, 8);
    ppm_class = maxIndex(y_pred + 9, 3);
    
    //! debug for class value =^.^=
    //printArray(y_pred, 13);
    
    Serial.print("PPM CO PREDICT: ");
    Serial.println(ppm_value);
    Serial.print("TEMP/HUMID CLASS: ");
    Serial.println(temp_humidity_class);
    Serial.print("PPM CLASS: ");
    Serial.println(ppm_class); 

    display->print("PPM CO PREDICT: ");
    display->println(ppm_value);
    display->print("TEMP/HUMID CLASS: ");
    display->println(temp_humidity_class);
    display->print("PPM CO CLASS: ");
    display->println(ppm_class);
    display->display();


    // allert with bot every 2 hours
    static unsigned long lastDetectionTime = 0;
    unsigned long currentMillis = millis();

    if (lastDetectionTime == 0) {
        if (ppm_value > 760 || ppm_class >= 3 || temp_humidity_class >= 6) {
            SendAllertMsgBot();
        }
        lastDetectionTime = currentMillis;
    }
    if (currentMillis - lastDetectionTime >= 7200000) { // 7200000 ms = 2 hours
        if (ppm_value > 760 || ppm_class >= 3 || temp_humidity_class >= 6) {
            SendAllertMsgBot();
        }
        lastDetectionTime = currentMillis;
    }    
}

void onBtn1Released(uint8_t pinBtn){
    deviceInfo = !deviceInfo;
}

void setup() {
    IoTBoard::init_serial();
    IoTBoard::init_leds();
    IoTBoard::init_buttons();
    IoTBoard::init_display();
    display->println(F("Display enabled"));
    display->display();

    IoTBoard::init_spi();
    display->println(F("SPI (HSPI) enabled"));
    display->display();

    if (IoTBoard::init_lora()) {
        lora->onReceive(onLoRaReceive);
        lora->onTxDone(onLoRaSend);
        lora->receive();
        display->println(F("LoRa enabled"));
    } else {
        digitalWrite(LED_RED, HIGH);
        display->printf(F("LoRa Error"));
    }
    display->display();
    
    if(loadCtrlMailBoxCredentials()){
        Serial.println("=^.^=");
        display->println(F("=^.^="));
        display->display(); 
    }
    delay(2000);

    buttons->onBtn1Release(onBtn1Released);

    pinMode(LED_RED, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);

    net_ssl.setInsecure(); 
    
    // try to load the credentials
    if(loadWiFiCredentials()) {
        digitalWrite(LED_GREEN, HIGH);

        // if the credentials have been loaded, try connecting to Station mode
        display->clearDisplay();
        display->setCursor(0, 0);
        display->println(F("Attempt to connect to the Wi-Fi saved network ..."));
        display->display();   
        configureWiFi(WIFI_AP_STA);
    } else {
        // start in AP mode to allow the initial configuration
        display->clearDisplay();
        display->setCursor(0, 0);
        display->println(F("AP mode, configure wifi!"));
        display->display();   
        configureWiFi(WIFI_AP);
    }
    delay(1000);

    if(WiFi.status() == WL_CONNECTED && loadChatDetails() && loadAccountCredentials()){
        bot_started = true;
        bot_active = true;
        // send message passing the `/restart` command
        SendRestartMessageBot();
    }

    // init dht11 sensor
    dht.begin();

    // init AI model
    if (!ml.begin(model_data)) {
        Serial.println("Error in loading the model!");
        while (true);
    }

}

void loop() {
    buttons->update();
    // WiFi auto-configuration
    dnsServer.processNextRequest();
    if (firstConnection && WiFi.status() != WL_CONNECTED && WiFi.getMode() == WIFI_AP_STA) {
        digitalWrite(LED_RED, HIGH);
        digitalWrite(LED_GREEN, LOW);

        WiFi.begin(ssid, password);
        delay(1000);
        display->clearDisplay();
        display->setCursor(0,0);
        display->println(F("Connecting to WiFi..."));
        display->display();
        connected = false;
        bot_active = false;
    }

    if (WiFi.status() == WL_CONNECTED && connected == false) {
        digitalWrite(LED_RED, LOW);
        digitalWrite(LED_GREEN, HIGH);

        display->clearDisplay();
        display->setCursor(0,0);
        display->println(F("Connected to WiFi"));
        display->println("SSID: " + ssid);
        display->display();
        connected = true;
        
        if(!bot_active && loadChatDetails() && loadAccountCredentials()){
            bot_active = true;
            // send message passing the "/restart" command
            SendRestartMessageBot();
        }
    }

    // message received from telegram bot (TonyBot)
    if (!lora_priority){
        // download messages received every (Bot_lasttime + Bot_mtbs)ms
        if (connected && millis() > Bot_lasttime + Bot_mtbs) {
            int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
            while (numNewMessages) {
                for (int i = 0; i < numNewMessages; i++) {
                    handleNewMessages(bot.messages[i]);
                }
                numNewMessages = bot.getUpdates(bot.last_message_received + 1);
            }
            Bot_lasttime = millis();
        }
    } 

    // reaction to the LoRa incorrect messages received
    if (loraFlagError){
        loraFlagReceived = false;
        display->display();
        sendMessageLoRa("NACK");
        delay(100);
        digitalWrite(LED_RED, LOW);
    }
    // reaction to the LoRa correct messages received
    if(loraFlagReceived){
        loraFlagReceived = false;
        display->display();
        sendMessageLoRa("ACK");
        delay(100);
        handlerSendMessage();
        delay(100);
    }  
    

    // detection and prediction value wiht AI model every 10s
    static unsigned long lastDetectionTime = 0;
    unsigned long currentMillis = millis();

    if (lastDetectionTime == 0) {
        DetectionAndPrediction();
        display->printf("MT addr:  %04X\n", localAddress);
        display->println("MT KEY: " + MAILTON_KEY );
        display->display();
        lastDetectionTime = currentMillis;
    }
    if (currentMillis - lastDetectionTime >= 10000) {
        DetectionAndPrediction();
        display->setCursor(0,0);
        display->printf("MT addr:  %04X\n", localAddress);
        display->println("MT KEY: " + MAILTON_KEY );
        display->display();
        lastDetectionTime = currentMillis;
    }
    
    if (deviceInfo) {
        digitalWrite(LED_GREEN, LOW);
        display->clearDisplay();
        display->setCursor(0,0);
        display->printf("MT addr:  %04X\n", localAddress);
        display->println("MT KEY: " + MAILTON_KEY );
        display->printf("CMB addr: %04X\n",ctrlmbAddress);
        display->println("CMB Name: " + CTRLMAILBOX_NAME);
        display->println("CMB KEY: " + CTRLMAILBOX_KEY);
        display->display();
        digitalWrite(LED_GREEN, HIGH);
    }
}
