/**
 * @file mailTon.cpp
 * @author Alessandro Ferrante
 * @brief 
 * @version 1.9.0
 * @date 2025-03-05
 * 
 * @copyright 2025
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
 
// Dichiara l'oggetto server (porta 80)
AsyncWebServer server(80);
DNSServer dnsServer;
const byte DNS_PORT = 53;

const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <title>Configurazione Wi-Fi</title>
</head>
<body>
  <h1>Imposta la tua rete Wi-Fi</h1>
  <form action="/" method="POST">
    <label for="ssid">Nome Rete (SSID):</label><br>
    <input type="text" id="ssid" name="ssid" required><br><br>
    <label for="password">Password:</label><br>
    <input type="password" id="password" name="password" required><br><br>
    <input type="submit" value="Salva">
  </form>
</body>
</html>
)rawliteral";

// configurazione wifi
void configureWiFi(wifi_mode_t mode);
String ssid;
String password;
bool firstConnection = false;
bool connected = false;
// file per salvare le credenziali
const char* credentialsFile = "/wifi.json";

// Bot telegram
WiFiClientSecure net_ssl;
UniversalTelegramBot bot("telegramToken", net_ssl);
int Bot_mtbs = 200;
long Bot_lasttime;
String chat_id;
String from_name;

// Funzione per salvare le credenziali con Preferences
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
  
// Funzione per caricare le credenziali con Preferences
bool loadCredentials() {
    Preferences preferences;
    preferences.begin("wifi", true); // true per modalità di sola lettura
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

// Funzione Callback per la pagina principale
void handleRoot(AsyncWebServerRequest *request) {
    request->send(200, "text/html", htmlPage);
}

// Funzione Callback per gestire il form di salvataggio
void handleSave(AsyncWebServerRequest *request) {
    digitalWrite(LED_GREEN, LOW);
    // Verifica e leggi i parametri inviati
    if(request->hasParam("ssid", true) && request->hasParam("password", true)) {
        ssid = request->getParam("ssid", true)->value();
        password = request->getParam("password", true)->value();
        
        //Serial.println("Dati ricevuti:");
        //Serial.println("SSID: " + ssid);
        //Serial.println("Password: " + password);
        
        // usa Preferences per salvare 
        if (saveCredentials(ssid, password)) {
            request->send(200, "text/html", "<h2>Dati salvati correttamente!</h2>");
        } else {
            request->send(500, "text/html", "<h2>Errore nel salvataggio dei dati</h2>");
        }
        
        // Dopo aver salvato le credenziali, cambia modalità a STA
        display->println("Dati Salvati");
        display->println("SSID: " + ssid);
        display->println("Password: " + password);
        display->display();
        delay(2000);
        configureWiFi(WIFI_AP_STA);
    } else {
        request->send(400, "text/html", "Errore nella ricezione dei dati");
    }
}

void configureWiFi(wifi_mode_t mode) {
    switch (mode) {
        case WIFI_AP:
            // Configura il modulo come Access Point
            WiFi.softAP("GatewayConfigAP", NULL);
            //WiFi.onEvent(on_connected_station, ARDUINO_EVENT_WIFI_AP_STACONNECTED);
            
            // il DNS server reindirizza tutte le richieste a WiFi.softAPIP()
            dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

            // Configura le rotte del server
            server.on("/", HTTP_GET, handleRoot);
            server.on("/", HTTP_POST, handleSave);
            server.begin();
            //Serial.println("HTTP server started");
            break;
        case WIFI_AP_STA:
            // Configurazione modalità AP
            WiFi.disconnect(true);
            WiFi.mode(WIFI_AP_STA);
            WiFi.softAP("GatewayConfigAP", NULL);
            dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
            server.on("/", HTTP_GET, handleRoot);
            server.on("/", HTTP_POST, handleSave);
            server.begin();

            // Configura il modulo in modalità Station
            WiFi.begin(ssid, password);
            
            display->clearDisplay();
            display->setCursor(0,0);
            display->println("Connecting to WiFi...");
            display->display();
            
            unsigned long startAttemptTime = millis();
            const unsigned long timeout = 5000; // 5 secondi di timeout
            
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

void handleNewMessages(telegramMessage m) {
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
        bot.sendMessage(chat_id, "Ciao " + from_name + "! Sono MailTon 📬🔔 un bot Telegram su Arduino!", "");
    }

    // TODO
    /*if (text == "/configureWifi") {
        bot.sendMessage(chat_id, "OK. Send me the new SSDI of your WiFi with the /newwWiFiSSID command and the password of your WiFi with /newWiFiPassword", "");
    }*/
}

void handlerSendMessage(const String &msg) {
    bot.sendMessage(chat_id, msg, "");
}

void setup() {
    IoTBoard::init_serial();
    IoTBoard::init_display();
    IoTBoard::init_leds();
    display->println("Display enabled");
    display->setCursor(0,0);
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
}

void loop() {
    dnsServer.processNextRequest();
    if (firstConnection && WiFi.status() != WL_CONNECTED && WiFi.getMode() == WIFI_AP_STA) {
        WiFi.begin(ssid, password);
        delay(1000);
        display->clearDisplay();
        display->setCursor(0,0);
        display->println("Connecting to WiFi...");
        display->display();
        connected = false;
    }
    if (WiFi.status() == WL_CONNECTED && connected == false) {
        display->clearDisplay();
        display->setCursor(0,0);
        display->println("Connected to WiFi");
        display->println("SSID: " + ssid);
        //!display->println("IP Address: " + WiFi.localIP());
        display->display();
        connected = true; // Reset the flag to avoid repeating the message

    }
    // scarica i messaggi ricevuti ogni 200ms
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
