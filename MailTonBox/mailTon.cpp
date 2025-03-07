/**
 * @file mailTon.cpp
 * @author Alessandro Ferrante (github@alessandroferrante)
 * @brief 
 * @version 2.4.0
 * @date 2025-03-05
 * 
 * @copyright Copyright (c) 2025
 *
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
 
// server object port 80 
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
                    <p>Il dispositivo <b style="color: #0bd43a;">mailTon</b> è il componente principale del sistema IoT per la tua cassetta postale. Segui queste istruzioni per configurarlo correttamente:</p>
                    <ul>
                        <li><b style="color: #0bd43a;">Connessione WiFi</b>: Assicurati che mailTon sia connesso alla tua rete WiFi.</li>
                        <li><b style="color: #0bd43a;">Notifiche Telegram</b>: Una volta connesso, mailTon potrà inviare notifiche tramite Telegram.</li>
                        <li><b style="color: #0bd43a;">Avvio del Bot Telegram</b>: Avvia il bot Telegram utilizzando il comando `/start`. Il bot riceverà e risponderà ai tuoi messaggi, mantenendo una sessione attiva anche in caso di riavvio, grazie al salvataggio del <i>`chat_id`</i> in memoria.</li>
                        <li><b style="color: #0bd43a;">Cambio Credenziali WiFi</b>: In futuro, potrai cambiare le credenziali WiFi anche tramite il bot Telegram.</li>
                    </ul>
            </div>
        </div>
        <div id="savedCredentials" class="modal">
            <div class="modal-content">
                <a href="javascript:void(0)" onclick="closeModalCredentials()">X</a>
                <h2 style="color: #0bd43a;">Dati salvati correttamente!</h2>
            </div>
        </div>
        <div id="ErrorCredentials" class="modal">
            <div class="modal-content">
                <a href="javascript:void(0)" onclick="closeModalCredentials()">X</a>
                <h2 style="color: red;">Errore nel salvataggio dei dati</h2>
            </div>
        </div>
    </div>
    <footer class="footer">
        <p>© 2023 Powered by <a href="https://alessandroferrante.net/">Alessandro Ferrante</a>. All rights reserved.</p>
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
UniversalTelegramBot bot("telegramToken", net_ssl);
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

// LoRa variables
int count = 0;
int count_sent = 0;
bool loraFlagReceived = false;
uint16_t localAddress = 0x02;     
uint16_t destination = 0xFF;
String lora_msg = ""; // payload of packet

// save credentials with Preferences
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

void on_connected_station(WiFiEvent_t event, WiFiEventInfo_t info) {
    display->println("ConnectionAP");
    display->print("Mac address of the client: ");
    display->printf(MACSTR "\n", MAC2STR(info.wifi_ap_staconnected.mac));
    display->println(IPAddress(info.wifi_ap_staconnected.aid));
    display->display();
}

// Callback for the main page
void handleRoot(AsyncWebServerRequest *request) {
    request->send(200, "text/html", htmlPage);
}

// Callback To manage the form
void handleSave(AsyncWebServerRequest *request) {
    digitalWrite(LED_RED, LOW);

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
        
        // Dopo aver salvato le credenziali, cambia modalità a STA
        display->println("Dati Salvati");
        display->println("SSID: " + ssid);
        display->println("Password: " + password);
        display->display();
        delay(2000);
        configureWiFi(WIFI_AP_STA);
    } else {
        request->send(400, "text/html", "<h2>Errore nella ricezione dei dati</h2>");
    }
}

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
                //!display->println("IP Address: " + WiFi.localIP());
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

bool loadChatDetails() {
    Preferences preferences;
    preferences.begin("telegram", true);  // true -> sola lettura
    savedChatID = preferences.getString("chat_id", "");
    savedUsername = preferences.getString("username", "");
    preferences.end();

    // Chat ID not found
    if (savedChatID.isEmpty()) 
        return false;
    
    display->println("Chat ID loaded:");
    display->println(savedChatID);
    display->println("Username loaded:");
    display->println(savedUsername);
    display->display();
    return true;
}

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
        bot.sendMessage(chat_id, "Hello " + from_name + "! I am Mailton 📬🔔 a bot telegram on Arduino! I will see you when it comes mail!");
    }
    // ignored message for unauthorized user
    if (savedChatID != chat_id)
        return;

    if (text == "/reboot") {
        bot.sendMessage(chat_id, "Ciao " + from_name + " Arduino è stato riavviato, ma adesso sono di nuovo attivo!🚀");
    } else if (text == "/description") {
        bot.sendMessage(chat_id, "🚀");
    } else if (text == "/about") {
        bot.sendMessage(chat_id, "🚀");
    } else if (text == "/commands") {
        bot.sendMessage(chat_id, "Available commands:\n/start - Start the bot\n/reboot - Reboot the bot\n/description - Get a description\n/about - About this bot\n/commands - List all commands \n/configurewifi - Configure your WiFi");
    } else if (text == "/configurewifi") {
        bot.sendMessage(chat_id, "OK. Send me the new SSDI of your WiFi", "");
        botConfigureWiFi = true;
        newWIFIbyBot = true;
        return;
    } else if(!botConfigureWiFi){
        bot.sendMessage(chat_id, "Unknown command. Type /commands to see the list of available commands.");
    }
    
    if (botConfigureWiFi){
        if (newWIFIbyBot){
            ssid = text;
            bot.sendMessage(chat_id, "OK. Now send me the password of your WiFi", "");
            newWIFIbyBot = false;
            newPASSbyBot = true;
            return;
        } 
        if (newPASSbyBot){
            bot.sendMessage(chat_id, "OK. Saved credentials! Now wait for the connection restart.", "");
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

void handlerSendMessage() {
    display->clearDisplay();
    display->setCursor(0,0);
    
    if(bot_active){
        if (lora_msg  == "Mailbox Opened") {
            bot.sendMessage(chat_id, "Mailbox Opened, someone is already withdrawing the mail 📭");
            display->println("Send to bot: mail in the mailbox!");
            display->display();
        }else if (lora_msg == "New Mail"){
            bot.sendMessage(chat_id, "Mail in the mailbox!📬");
            display->println("Send to bot: mail in the mailbox!");
            display->display();
        } else {
            bot.sendMessage(chat_id, lora_msg);
            display->println("Lora_msg : " + lora_msg);
            display->display();
        }
    }else{
        display->println("Bot not activated, not sent notification");
        display->display();
    }
}

void SendRebootMessageBot(){
    telegramMessage rebootMsg;
    rebootMsg.chat_id = savedChatID;
    rebootMsg.text = "/reboot";         
    rebootMsg.from_name = savedUsername;   
    // call the HandleNewMessages() by passing the reboot message
    handleNewMessages(rebootMsg);
}

void onLoRaSend() {
    /*display->clearDisplay();
    display->setCursor(0,0);
    display->printf("LoRa Send: %d \n", count_sent);
    display->display();
    */
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

    while (lora->available()){          // can't use readString() in callback, so
        incoming += (char)lora->read(); // add bytes one by one
    }

    if (incomingLength != incoming.length()){
        display->clearDisplay();
        display->setCursor(0,0);
        display->println("error: message length does not match length");
        display->display();
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

    lora_msg = incoming.c_str();
    loraFlagReceived = true;
}

void setup() {
    IoTBoard::init_serial();
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
    
    delay(2000);

    pinMode(LED_RED, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_BLUE, OUTPUT);

    digitalWrite(LED_BLUE, HIGH);
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_GREEN, HIGH);
    
    net_ssl.setInsecure(); 
    
    // Prova a caricare le credenziali
    if(loadCredentials()) {
        // Se le credenziali sono state caricate, prova a connetterti in modalità Station
        display->clearDisplay();
        display->setCursor(0, 0);
        display->println("Tentativo di connessione alla rete Wi-Fi salvata...");
        display->display();   
        configureWiFi(WIFI_AP_STA);
 
    } else {
        // Avvia in modalità AP per permettere la configurazione iniziale
        display->clearDisplay();
        display->setCursor(0, 0);
        display->println("AP mode, configure wifi!");
        display->display();   
        configureWiFi(WIFI_AP);
    }
    delay(1000);

    if(WiFi.status() == WL_CONNECTED && loadChatDetails()){
        bot_active = true;
        // send message passing the /reboot command
        SendRebootMessageBot();
    }
}

void loop() {
    // WiFi configuration
    dnsServer.processNextRequest();
    if (firstConnection && WiFi.status() != WL_CONNECTED && WiFi.getMode() == WIFI_AP_STA) {
        WiFi.begin(ssid, password);
        delay(1000);
        display->clearDisplay();
        display->setCursor(0,0);
        display->println("Connecting to WiFi...");
        display->display();
        connected = false;
        bot_active = false;
    }

    if (WiFi.status() == WL_CONNECTED && connected == false) {
        display->clearDisplay();
        display->setCursor(0,0);
        display->println("Connected to WiFi");
        display->println("SSID: " + ssid);
        //!display->println("IP Address: " + WiFi.localIP());
        display->display();
        connected = true;
        
        if(!bot_active && loadChatDetails()){
            bot_active = true;
            // send message passing the "/reboot" command
            SendRebootMessageBot();
        }
    }

    // download messages received every 10000ms
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

    if(loraFlagReceived){
        loraFlagReceived = false;
        display->display();
        //delay(100);
        handlerSendMessage();
    }
    delay(500);
}
