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

Preferences preferences;

// WiFi configuration
void configureWiFi(wifi_mode_t mode);
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
UniversalTelegramBot bot(telegramToken, net_ssl);
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
uint16_t destination = 0;
String lora_msg = ""; // payload of packet
bool lora_priority = false;

String MAILTON_KEY =  "00BKFWR39FN48FN40GM30DM69GJ"; // random
String CTRLMAILBOX_NAME = "CMBX1";
String CTRLMAILBOX_KEY = "VSJ5KNVS903NJ7N12NN";

const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <meta charset="utf-8">
    <title>Mailton configuration</title>
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
        ::-webkit-scrollbar {
        display: none;
        }      
        body {
        padding: 0;
        margin: 0;
            font-family: 'Poppins', sans-serif;
            background-color: var(--background-neumorphism);
            background-repeat: repeat;
            overflow-x: hidden;
        }
        #home{
            display: flex;
            flex-direction: row;
            gap: 50px;
            justify-content: center;
            align-items: center;
            width: 100%;
            min-height: max-content;
            height: 100vh;
        }
        .topnav {
            position: absolute;
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
            display: flex;
            flex-direction: row;
            color: var(--font-green);
            padding: 40px 30px;
            height: max-content;
            width: fit-content;
            gap: 100px;
            text-align: center;
            margin-top: 80px;
            margin-bottom: 20px;
            border-radius: 25px;
            transition: 0.6s ease-out;
            background: linear-gradient(315deg, var(--first-gradient), var(--second-gradient));
            box-shadow:  -5px -5px 10px var(--up-box),
                5px 5px 10px var(--down-box);
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
            min-width: 230px;
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
        .footer {
            position: absolute;
            bottom: 0px;
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
            margin-top: 80px;
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

        @media only screen and (max-width: 1000px) {
            #home{
                flex-direction: column;
            }
            .container {
                width: 300px;
                margin-top: 100px;
                margin-bottom: 0;
                display: flex;
                flex-direction: column;

            }
            input[type="text"], input[type="password"], input[type="submit"] {
                width: 100%;
            }
            .footer{
                bottom: 0;
                position: relative;
            }
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
    <div id="home">
        <div class="container">
            <form id="dataForm">
                <div>
                    <h1>Set your Wi-Fi</h1>
                    <label for="ssid">SSID*:</label><br>
                    <input type="text" placeholder="Enter SSID your WiFi" id="ssid" name="ssid" required>
                    <label for="password">Password*:</label><br>
                    <input placeholder="Enter password your WiFi" type="password" id="password" name="password" required><br><br>
                </div>
                <input type="submit" value="Save">
            </form>
        </div>
        <div class="container">
            <form id="wifiForm">
                <div>
                    <h1>Set your MailTon</h1>
                    <!--
                    <label for="mailtonkey">MailTon KEY*:</label><br>
                    <input placeholder="Enter your MailTon KEY" type="password" id="mailtonkey" name="mailtonkey" required><br>
                    -->
                    <label for="Username">Username*:</label><br>
                    <input type="text" placeholder="Enter your Username" id="username" name="username" required>
                    <label for="user_password">Password*:</label><br>
                    <input placeholder="Enter your password" type="password" id="user_password" name="user_password" required><br>
                    <label for="token">Telegram Token: <small><i>(optional)</i></small></label><br>
                    <input type="text" value="Enter your Telegram Token" placeholder="Enter your Telegram Token" id="token" name="token"><br><br>
                </div>
                <input type="submit" value="Save">
         </form>
        </div>
        <div class="container">
            <form id="deviceForm">
                <div>
                    <h1>Add your CtrlMailBox</h1>
                    <label for="ctrlmailboxkey">CtrlMailBox KEY*:</label><br>
                    <input placeholder="Enter your CtrlMailBox KEY" type="password" id="ctrlmailboxkey" name="ctrlmailboxkey" required><br>
                    <label for="ctrlmailboxname">CtrlMailBox Name*:</label><br>
                    <input type="text" placeholder="Enter your CtrlMailBox Name" id="ctrlmailboxname" name="ctrlmailboxname" required>
                    <label for="ctrlmailboxaddress">CtrlMailBox address*:</label><br>
                    <input type="text" id="ctrlmailboxaddress" name="ctrlmailboxaddress" placeholder="0xABCD" pattern="0x[0-9A-Fa-f]{1,4}" required>
                </div>
                <input type="submit" value="Save">
         </form>
        </div>
        <div class="telegram-bot">
            <h1>Connect with our Telegram Bot</h1>
            <p>TonyBot helps you manage your mailbox, send notifications, and never miss important mail! 📬🚀 </p>
            <a href="https://t.me/MailTonyBot" target="_blank">Click here to connect</a>
        </div>
        <div id="aboyModal" class="modal">
            <div class="modal-content">
                <a href="javascript:void(0)" onclick="openModal()">X</a>
                    <h1>About MailTon</h1>
                    <p>The device <b style="color: #0bd43a;">MailTon</b> It is the main component of the IoT system for your mailbox. Follow these instructions to configure it correctly:</p>
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
        document.getElementById('dataForm').addEventListener('submit', function(event) {
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
        document.getElementById('deviceForm').addEventListener('submit', function(event) {
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

    display->println("Total reset: all configurations have been deleted!");
    display->display();
    
    // restart the device
    ESP.restart();
}
    
bool saveCtrlMailBoxCredentials (const String &newCtrlMailBox_key, const String &newCtrlMailBox_name, uint16_t &newCtrlMailBox_address) {
    preferences.begin("CtrlMailBox", false);
    preferences.putString("CtrlMailBox_key", newCtrlMailBox_key);
    preferences.putString("CtrlMailBox_name", newCtrlMailBox_name);
    preferences.putUShort("CtrlMailBox_address", newCtrlMailBox_address);

    preferences.end();

    display->println("Saved Account credentials:");
    display->println("CtrlMailBox key: " + newCtrlMailBox_key);
    display->println("CtrlMailBox name: " + newCtrlMailBox_name);
    display->println("CtrlMailBox address: " + newCtrlMailBox_address);
    display->display();
    return true;
}

// load CtrlMailBox data with Preferences
bool loadCtrlMailBoxCredentials() {
    preferences.begin("CtrlMailBox", true);
    CTRLMAILBOX_KEY = preferences.getString("CtrlMailBox_key", "");
    CTRLMAILBOX_NAME = preferences.getString("CtrlMailBox_name", "");
    destination = preferences.getUShort("CtrlMailBox_address", 0);
    preferences.end();

    if (CTRLMAILBOX_KEY.isEmpty() || CTRLMAILBOX_NAME.isEmpty() || destination == 0) {
        Serial.println("Account credentials don't find");
        return false;
    }

    display->println("Account credentials loaded:");
    display->println("CtrlMailBox key: " + CTRLMAILBOX_KEY);
    display->println("CtrlMailBox name: " + CTRLMAILBOX_NAME); 
    display->println("CtrlMailBox address: " + destination); 
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
    request->send(200, "text/html", htmlPage);
    
    //request->send(SPIFFS, "/index.html", "text/html");

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
            destination = strtol(inputAddress.c_str(), NULL, 16);  // Converte da stringa esadecimale a uint16_t
        } else {
            destination = inputAddress.toInt();  // Prova a convertirlo in decimale
        }

        // use preferences to save
        if (saveCtrlMailBoxCredentials(CTRLMAILBOX_KEY, CTRLMAILBOX_NAME, destination)) {
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
            // configure the form as an access point
            WiFi.softAP(ssidAP, passwordAP);
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
            WiFi.softAP(ssidAP, passwordAP);
            
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

/**
 * @brief manages the messages received from a telegram bot (TonyBot).
 * Depending on the command received, 
 * (/start, /restart, /description, /about, /commands, /configurewifi, /ai, /allert_ai), 
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

    // Estrai nome, key e dati
    String senderName = extractValue(incoming, "NAME");
    String senderKey = extractValue(incoming, "CTRLMAILBOX_KEY");
    String receiverKey = extractValue(incoming, "MAILTON_KEY");
    String data = extractValue(incoming, "DATA");

    // Controllo autenticazione
    //Preferences preferences;
    //preferences.begin("account", true);
    //String expectedSenderKey = preferences.getString("CTRLMAILBOX_KEY", "");
    //preferences.end();
    String expectedSenderName = CTRLMAILBOX_NAME; // todo da salvare in prefereces
    String expectedSenderKey = CTRLMAILBOX_KEY; // todo da salvare in prefereces
    String expectedReceiverKey = MAILTON_KEY;

    if (senderName != expectedSenderName || senderKey != expectedSenderKey || receiverKey != expectedReceiverKey) {
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
    String senderName = CTRLMAILBOX_NAME;
    String senderKey = MAILTON_KEY;
    String receiverKey = CTRLMAILBOX_KEY;

    //String senderKey = preferences.getString("MAILTON_KEY", "");  
    //preferences.end();

    // Componi il messaggio nel formato: NAME=XYZ;KEY=ABC;DATA=Messaggio
    String messageToSend = "NAME=" + senderName + ";CTRLMAILBOX_KEY=" + receiverKey  + ";MAILTON_KEY=" + senderKey + ";DATA=" + loraSendMsg;

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
    loadCtrlMailBoxCredentials(); // load Key, name and address of the CtrlMailBox

    delay(2000);

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
        lastDetectionTime = currentMillis;
    }
    if (currentMillis - lastDetectionTime >= 10000) {
        DetectionAndPrediction();
        lastDetectionTime = currentMillis;
    }

}
