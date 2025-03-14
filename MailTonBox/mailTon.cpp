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
 * @version 3.1
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
#define INPUT_MAX_3 1500.0

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

const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <meta charset="utf-8">
    <title>Configurazione Wi-Fi</title>
    <style>
        :root{
            --font-green: #1A6167;
            --background-neumorphism: #032D32;/* #0d3d2a*/
            --first-gradient: #032D32;
            --second-gradient: #033237;
            --up-box: #032529; /*#0a3424  #08251a*/
            --down-box: #033D42;  /*#12553a  00442f*/
            --footer: #033D42;
            --footer-font: #032529;
        }
        body {
            font-family: 'Poppins', sans-serif;
            height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
            background-color: var(--background-neumorphism);
            background-repeat: repeat;
            overflow: hidden;
        }
        #home{
            display: flex;
            flex-direction: column;
            align-items: center;
            width: 100%;
        }
        .topnav {
            top:0px;  
            position: fixed;
            width: 90%;
            margin-top: 10px;
            height: 50px;
            left: 50%;
            transform: translateX(-50%);
            border-radius: 35px;
            color: #3399ff;
            text-align: center;
            display: flex;
            align-items: center;
            justify-content: space-between;
            border-radius: 33px;
            background: linear-gradient(315deg, var(--first-gradient), var(--second-gradient));
            box-shadow:  -4px -4px 8px var(--up-box),
                4px 4px 8px var(--down-box);
            
        }
        .topnav h1{
            font-size: 20px;
            margin: 0;
            margin-left: 20px;
        }
        .topnav p{
            margin: 0;
            margin-top: 6px;
            margin-left: 5px;
            font-size: 11px;
        }
        .topnav .title{
            margin: 0;
            display: flex;
            flex-direction: row;
        }
        .topnav .right-links {
            display: flex;
            align-items: center;
            margin-right: 20px;
        }
        .topnav a {
            color: rgba(240, 248, 255, 0.671);
            text-decoration: none;
            font-size: 16px;
            font-weight: 800;
            margin-left: 10px;
        }
        .topnav .left-links {
            display: flex;
            align-items: center;
        }
        .container {
            color: var(--font-green);
            padding: 40px 30px;
            width: 300px;
            text-align: center;
            margin-top: 80px;
            border-radius: 25px;
            transition: 0.6s ease-out;
            background: linear-gradient(315deg, var(--first-gradient), var(--second-gradient));
            box-shadow:  -5px -5px 10px var(--up-box),
                5px 5px 10px var(--down-box);
            margin-bottom: 50px;
        }
        .container h1 {
            color: #27873D;
            font-size: 20px;
            margin-top: 0;
        }
        .container label {
            text-align: left;
            margin-left: 25px;
            font-size: 15px;
            display: block;
            font-weight: 600;
            color: var(--font-green);
        }
        input[type="text"], input[type="password"] {
            width: 100%;
            padding: 10px;
            margin-bottom: 20px;
            border: 1px solid #ccc;
            border-radius: 25px;
            box-sizing: border-box;
            background: #062b4100;
            color: var(--font-green);
            width: 80%;
            font-size: 14px;
            font-weight: 500;
            text-transform: capitalize;
            border: 0;
            outline: none;
            padding-left: 32px;   
            box-shadow: var(--down-box) 4px 4px 6px inset, 
                        var(--up-box)  -4px -4px 6px inset;
        }
        input[type="submit"] {
            width: 100%;
            padding: 10px;
            background-color: var(--background-neumorphism);
            box-shadow: var(--down-box)  4px 4px 6px, 
                        var(--up-box) -4px -4px 6px;
            border: none;
            border-radius: 25px;
            color: #3399ff;
            font-weight: 800;
            font-size: 16px;
            cursor: pointer;
        }
        input[type="submit"]:hover {
            box-shadow: var(--down-box) 3px 3px 6px inset, 
                        var(--up-box) -3px -3px 6px inset;
        }
        input:active, input:hover {
            border: none;
        }
        input::placeholder{
            font-size: 14px;
            font-weight: 500;
            color: var(--font-green);
            text-transform: capitalize;
        }
        @media only screen and (max-width: 600px) {
            .container {
                width: 300px;
                margin-top: 90px;
            }
            input[type="text"], input[type="password"], input[type="submit"] {
                width: 100%;
            }
        }
        .footer {
            position: fixed;
            bottom: 0;
            width: 100%;
            text-align: center;
            justify-content: center;
            height: min-content;
            padding: 10px 0px;
            font-size: 10px;
            color: var(--footer-font);
            background-color: var(--footer);
            font-family: 'Poppins', sans-serif;
            font-weight: 600;
        }
        .footer a {
            color: var(--footer-font);
        }
        
        .telegram-bot {
            padding: 30px 20px;
            font-weight: bold;
            font-size: 15px;
            text-decoration: none;
            color: var(--font-green);
            width: 300px;
            border-radius: 25px;
            margin-bottom: 100px;
            transition: 0.6s ease-out;
            background: linear-gradient(315deg, var(--first-gradient), var(--second-gradient));
            box-shadow:  -5px -5px 10px var(--up-box),
                5px 5px 10px var(--down-box);
            text-align: center;
        }
        .telegram-bot a {
            font-weight: 600 ;
            font-size: 15px;
            text-decoration: none;
            color: #3399ff;
        }
        .telegram-bot h1 {
            font-size: 16px;
            color: #27873D;
        }
        #aboyModal, 
        #savedCredentials, 
        #ErrorCredentials {
            display: none;
        }
        .modal {
            position: fixed;
            z-index: 1;
            left: 0;
            top: 40px;
            width: 100%;
            height: 100%;
            overflow: hidden;
            overflow-y: auto;
        }
        .modal-content {
            color: aliceblue;
            font-weight: 700;
            font-size: 15px;
            font-weight: 500;
            background: #0c0c0c00;
            margin: auto;
            padding: 35px 45px;
            width: 60%;
            max-width: 500px;
            height: fit-content;
            backdrop-filter: blur(18px);
            -webkit-backdrop-filter: blur(18x);
            z-index: 1000;
            background: linear-gradient(145deg,#ffffff02, #0c0c0c00);;
            border: 1px solid #ffffff17;
            border-style: inset;
            backdrop-filter: blur(10px);
            -webkit-backdrop-filter: blur(10px);
            border-radius: 25px;
            margin-top: 60px;
            overflow-y: none;
            margin-bottom: 50px;
            align-items: center;
            align-content: center;
            justify-content: center;
        }
        .modal-content a {
            font-size: 14px;
            text-align: center;
            display: block;
            float: right;
            color: red;
            font-weight: 900;
            text-decoration: none;
            padding: 0px 5px;
            border-radius: 100%;
            border: 3px solid #ff0000;
            background-color: rgba(240, 248, 255, 0.014);
        }
        .modal-content h1{
            font-size: 24px;
            color: #2587ef;
            text-align: center;
        }
        .modal-content p{
            text-align: left;
        }
        .modal-content h2{
            font-size: 18px;
        }
    </style>
</head>
<body>
    <div id="home">
        <div class="topnav">
            <div class="title">
                <h1>MailTonBox</h1>
                <p>✉️ 🔔 📬</p>
            </div>  
            <div class="right-links">
                <a href="#home">Home</a>
                <a href="javascript:void(0)" onclick="openModal()">About</a>
                <a href="https://alessandroferrante.net/">Contact</a>
            </div>
        </div>
        <div class="container">
            <h1>Set your Wi-Fi</h1>
            <form id="wifiForm">
                <label for="ssid">SSID:</label><br>
                <input type="text" placeholder="Enter SSID your WiFi" id="ssid" name="ssid" required>
                <label for="password">Password:</label><br>
                <input placeholder="Enter password your WiFi" type="password" id="password" name="password" required><br><br>
                <input type="submit" value="Save">
            </form>
        </div>
        <div class="telegram-bot">
            <h1>Connect with our Telegram Bot</h1>
            <p>MailTonBot helps you manage your mailbox, send notifications, and never miss important mail! 📬🚀 </p>
            <a href="https://t.me/MailTonyBot" target="_blank">Click here to connect</a>
        </div>
        <div id="aboyModal" class="modal">
            <div class="modal-content">
                <a href="javascript:void(0)" onclick="openModal()">X</a>
                    <h1>About MailTon</h1>
                    <p>The device <b style="color: #0bd43a;">mailTon</b> It is the main component of the IoT system for your mailbox. Follow these instructions to configure it correctly:</p>
                    <ul>
                        <li><b style="color: #0bd43a;">WiFi connection</b>: Make sure that Mailton is connected to your WiFi network.</li>
                        <li><b style="color: #0bd43a;">Telegram notifications</b>: Once connected, Mailton can send notifications via Telegram.</li>
                        <li><b style="color: #0bd43a;">Start of Bot Telegram</b>: Start the Bot Telegram using the `/start` command. The bot will receive and respond to your messages, keeping an active session even in the event of restart, thanks to the rescue of the <i> `chat_id` </i> in memory.</li>
                        <li><b style="color: #0bd43a;">Wifi credential change</b>: In the future, you can also change wifi credentials through the bot telegram.</li>
                    </ul>
            </div>
        </div>
        <div id="savedCredentials" class="modal">
            <div class="modal-content">
                <a href="javascript:void(0)" onclick="closeModalCredentials()">X</a>
                <h2 style="color: #0bd43a;">Data saved correctly!</h2>
            </div>
        </div>
        <div id="ErrorCredentials" class="modal">
            <div class="modal-content">
                <a href="javascript:void(0)" onclick="closeModalCredentials()">X</a>
                <h2 style="color: red;">Error in saving data</h2>
            </div>
        </div>
    </div>
    <footer class="footer">
        <p>© 2025 Powered by <a href="https://alessandroferrante.net/">Alessandro Ferrante</a>. All rights reserved.</p>
    </footer>
    <script>
        let modalActive = false;
        function openModal() {
            const modal = document.getElementById("aboyModal");
            if (modalActive) {
                modal.style.display = "none";
                modalActive = false;
            } else {
                modal.style.display = "block";
                modalActive = true;
            }
        }
        document.getElementById('wifiForm').addEventListener('submit', function(event) {
            event.preventDefault(); // Evita il refresh della pagina

            let formData = new FormData(this); 
            
            fetch('/', {
                method: 'POST',
                body: formData
            })
            .then(response => response.text())
            .then(script => {
                eval(script);
            })
            .catch(error => console.error('Errore:', error));
        });
        function closeModalCredentials() {
            const modal1 = document.getElementById("savedCredentials");
            const modal2 = document.getElementById("ErrorCredentials");
            modal1.style.display = "none";
            modal2.style.display = "none";
        }
    </script>
</body>
</html>
)rawliteral";

// WiFi configuration
void configureWiFi(wifi_mode_t mode);
String ssid;
String password;
bool firstConnection = false;
bool connected = false;
// files to save credentials
const char* credentialsFile = "/wifi.json";

// Telegram bot
WiFiClientSecure net_ssl;
UniversalTelegramBot bot("7411303563:AAEcRU9EZebULA2VXfuVt-theJ8nRZf2ZpQ", net_ssl);
int Bot_mtbs = 200;
long Bot_lasttime;
String chat_id;
String from_name;
bool bot_active =  false;
String savedChatID = "";  
String savedUsername = "";
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
uint16_t destination = 0xFF;
String lora_msg = ""; // payload of packet
bool lora_priority = false;


// save WiFi credentials with Preferences
bool saveCredentials(const String &newSSID, const String &newPassword) {
    Preferences preferences;
    preferences.begin("wifi", false);
    preferences.putString("ssid", newSSID);
    preferences.putString("password", newPassword);
    preferences.end();

    display->println("Credenziali salvate:");
    display->println("SSID: " + newSSID);
    display->println("Password: " + newPassword);
    display->display();
    return true;
}

// load credentials with Preferences
bool loadCredentials() {
    Preferences preferences;
    preferences.begin("wifi", true);
    ssid = preferences.getString("ssid", "");
    password = preferences.getString("password", "");
    preferences.end();

    if (ssid.isEmpty() || password.isEmpty()) {
        Serial.println("Credenziali non trovate");
        return false;
    }

    display->println("Credenziali caricate:");
    display->println("SSID: " + ssid);
    display->println("Password: " + password); 
    display->display();
    return true;
}

// Callback for the main page
void handleRoot(AsyncWebServerRequest *request) {
    request->send(200, "text/html", htmlPage);
}

// Callback to manage the form
void handleSave(AsyncWebServerRequest *request) {

    // verify and read the parameters sent
    if(request->hasParam("ssid", true) && request->hasParam("password", true)) {
        ssid = request->getParam("ssid", true)->value();
        password = request->getParam("password", true)->value();
        
        // use preferences to save
        if (saveCredentials(ssid, password)) {
            request->send(200, "text", "document.getElementById('savedCredentials').style.display='block';");
        } else {
            request->send(500, "text", "document.getElementById('errorCredentials').style.display='block';");
        }        
        
        // after saving the credentials, changes to ap_sta modes
        display->println("Dati Salvati");
        display->println("SSID: " + ssid);
        display->println("Password: " + password);
        display->display();
        delay(2000);
        configureWiFi(WIFI_AP_STA);
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
            // configure the form as an access point
            WiFi.softAP("GatewayConfigAP", NULL);
            //WiFi.onEvent(on_connected_station, ARDUINO_EVENT_WIFI_AP_STACONNECTED);
            
            // the DNS Server redirects all requests to WiFi.softAPIP() and activate router management 
            dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

            // HTTP | configure the server routes
            server.on("/", HTTP_GET, handleRoot);
            server.on("/", HTTP_POST, handleSave);
            server.begin();
            
            break;
        case WIFI_AP_STA:
            // configuration AP mode
            WiFi.disconnect(true);
            WiFi.mode(WIFI_AP_STA);
            WiFi.softAP("GatewayConfigAP", NULL);
            dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
            server.on("/", HTTP_GET, handleRoot);
            server.on("/", HTTP_POST, handleSave);
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

/**
 * @brief manages the messages received from a telegram bot (TonyBot).
 * Depending on the command received, 
 * (/start, /reboot, /description, /about, /commands, /configurewifi, /ai, /allert_ai), 
 * performs different actions such as saving chat details, send reply messages,
 * configure wifi, or perform forecasts with the AI.
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

    if (text == "/start") {
        saveChatDetails(chat_id, from_name);
        bot.sendMessage(chat_id, "Hello " + from_name + "! I am TonyBot 🤖 a bot telegram on Mailton 📬🔔 (Ardunio)! \n\n When the mail arrives, PostaLino📪📣 will notify us. \nSee you when the mail arrives! 👀📨");
    }
    // ignored message for unauthorized user
    if (savedChatID != chat_id)
        return;

    if (text == "/reboot") {
        bot.sendMessage(chat_id, "Hello  " + from_name + "!🔔\n MailTon 📬🔔 has been restarted, but now I'm active again!🚀");
        botConfigureWiFi = false;
    } else if (text == "/description") {
        bot.sendMessage(chat_id, "🚀");
        botConfigureWiFi = false;
    } else if (text == "/about") {
        bot.sendMessage(chat_id, "🚀");
        botConfigureWiFi = false;
    } else if (text == "/commands") {
        bot.sendMessage(chat_id, "🎛️  Available commands:\n\n     ✅  /start - Start the bot\n\n     🔄️  /reboot - Reboot the bot\n\n     📑  /description - Get a description\n\n     📄  /about - About this bot\n\n     🕹️  /commands - List all commands \n\n     🛜  /configurewifi - Configure your WiFi \n\n     ❇️  /ai - Artificial Intelligence \n\n     ⚠️  /allert_ai - On/Off Alert notification from AI \n\n");
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
    }else if(!botConfigureWiFi){
        bot.sendMessage(chat_id, "Unknown command. Type /commands to see the list of available commands.");
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
            saveCredentials(ssid, password);
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
            display->println("Send to bot: Mailbox Opened");
            display->display();
        }else if (lora_msg == "New Mail"){
            static bool mFlag= false;
            if(mFlag) bot.sendMessage(chat_id, "🔔 Lino the Postino warned me that there is placed in the mailbox! 📬");
            else bot.sendMessage(chat_id, "🔔 PostaLino warned me that there is placed in the mailbox! 📬");
            mFlag = !mFlag;
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

// Reboot message, to reactivate the sending of messages to the bot
void SendRebootMessageBot(){
    telegramMessage rebootMsg;
    rebootMsg.chat_id = savedChatID;
    rebootMsg.text = "/reboot";         
    rebootMsg.from_name = savedUsername;   
    // call the HandleNewMessages() by passing the reboot message
    handleNewMessages(rebootMsg);
}

// Function to receive messages Lora
void onLoRaReceive(int packetSize) {
    lora_priority = true;
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
    
    if (incomingLength != incoming.length() || receivedChecksum != calculatedChecksum){ 
        display->clearDisplay();
        display->setCursor(0,0);
        display->println("error: message length or checksum does not match");
        loraFlagError = true;
        digitalWrite(LED_RED, HIGH);
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
    display->printf("Message: %s\n", incoming.c_str());
    display->printf("RSSI: %d\n", lora->packetRssi());
    display->printf("Snr: %02f\n", lora->packetSnr());
    
    lora->receive();

    if(!loraFlagError){    
        lora_msg = incoming.c_str();
    }

    loraFlagReceived = true;

    lora_priority = false;
}

// this function uses the interrupt when it is called
void onLoRaSend(){    
}

// send message LoRa (without interrupt)
void sendMessageLoRa(const String &loraSendMsg){
    digitalWrite(LED_RED, HIGH);
    String msg = loraSendMsg;
    lora->beginPacket();

    lora->write(destination);  // add destination address
    lora->write(localAddress); // add sender address
    lora->write(count_sent);   // add message ID
    lora->write(msg.length()); // add payload length
    
    byte checksum = 0;
    for (int i = 0; i < msg.length(); i++) {
        checksum ^= msg[i];
    }
    lora->write(checksum); // add checksum
    lora->print(msg); // add payload

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
    // Stampa ogni elemento dell'array
    for (int i = 0; i < size; i++) {
        message += "y_pred[" + String(i) + "] = " + String(arr[i]) + "\n";
    }
    bot.sendMessage(chat_id, message);
}
void printClassAndProbability(float* arr, int size) {
    int index = 0;
    float maxVal = arr[0];
    // Trova l'indice della classe con la probabilità massima
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
    display->clearDisplay();
    display->setCursor(0,16);
    float newT = dht.readTemperature();

    if (isnan(newT)) {
        display->println(F("I can't read the DHT sensor!"));
        display->display();
    } else {
    temperature = newT;
    display->print("Temperatura = ");
    display->println(temperature);
    display->display();
    }

    float newH = dht.readHumidity();

    if (isnan(newH)) {
    display->println(F("I can't read the DHT sensor!"));
    display->display();
    } else {
    humidity = newH;
    display->print("Humidity = ");
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
    
    //! debug for class value
    //printArray(y_pred, 13);
    
    //Serial.print("PPM Predetto: ");
    //Serial.println(ppm_value);
    //Serial.print("Classe Temp/Humidity: ");
    //Serial.println(temp_humidity_class);
    //Serial.print("Classe PPM: ");
    //Serial.println(ppm_class); 
    display->print("PPM Predetto: ");
    display->println(ppm_value);
    display->print("Classe Temp/Humidity: ");
    display->println(temp_humidity_class);
    display->print("Classe PPM: ");
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

void setup() {
    IoTBoard::init_serial();
    IoTBoard::init_leds();
    
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
    
    delay(2000);

    pinMode(LED_RED, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);

    net_ssl.setInsecure(); 
    
    // try to load the credentials
    if(loadCredentials()) {
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

    if(WiFi.status() == WL_CONNECTED && loadChatDetails()){
        bot_active = true;
        // send message passing the `/reboot` command
        SendRebootMessageBot();
    }

    // init dht11 sensor
    dht.begin();

    // init AI model
    if (!ml.begin(model_data)) {
        Serial.println("Errore nel caricamento del modello!");
        while (true);
    }

}

void loop() {
    
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
        
        if(!bot_active && loadChatDetails()){
            bot_active = true;
            // send message passing the "/reboot" command
            SendRebootMessageBot();
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
        lastDetectionTime = currentMillis;
    }
    if (currentMillis - lastDetectionTime >= 10000) {
        DetectionAndPrediction();
        lastDetectionTime = currentMillis;
    }

}
