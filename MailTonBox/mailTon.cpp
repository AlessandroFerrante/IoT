/**
 * @file main.cpp
 * @author Alessandro Ferrante
 * @brief 
 * @version 0.1
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


void configureWiFi(wifi_mode_t mode);
String ssid;
String password;
bool firstConnection = false;
bool connected = false;

void on_connected_station(WiFiEvent_t event, WiFiEventInfo_t info) {
    display->println("ConnectionAP");
    display->print("Mac address of the client: ");
    display->printf(MACSTR "\n", MAC2STR(info.wifi_ap_staconnected.mac));
    display->println(IPAddress(info.wifi_ap_staconnected.aid));
}

// Funzione callback per la pagina principale
void handleRoot(AsyncWebServerRequest *request) {
    request->send(200, "text/html", htmlPage);
}

// Funzione callback per gestire il form di salvataggio
void handleSave(AsyncWebServerRequest *request) {
    digitalWrite(LED_GREEN, LOW);
    // Verifica e leggi i parametri inviati
    if(request->hasParam("ssid", true) && request->hasParam("password", true)) {
        ssid = request->getParam("ssid", true)->value();
        password = request->getParam("password", true)->value();
        
        //Serial.println("Dati ricevuti:");
        //Serial.println("SSID: " + ssid);
        //Serial.println("Password: " + password);
        
        request->send(200, "text/html", "<h2>Dati salvati correttamente!</h2>");
        
        // Dopo aver salvato le credenziali, cambia modalità a STA
        display->println("Dati Salvati");
        display->println("SSID: " + ssid);
        display->println("Password: " + password);
        display->display();
        delay(2000);
        configureWiFi(WIFI_STA);
    } else {
        request->send(400, "text/html", "Errore nella ricezione dei dati");
    }
}

void configureWiFi(wifi_mode_t mode) {
    switch (mode) {
        case WIFI_AP:
            // Configura il modulo come Access Point
            WiFi.softAP("GatewayConfigAP", NULL);
            //Serial.println("Access Point avviato: GatewayConfigAP");
            //WiFi.onEvent(on_connected_station, ARDUINO_EVENT_WIFI_AP_STACONNECTED);
            
            // il DNS server reindirizza tutte le richieste a WiFi.softAPIP()
            dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

            // Configura le rotte del server
            server.on("/", HTTP_GET, handleRoot);
            server.on("/", HTTP_POST, handleSave);
            server.begin();
            //Serial.println("HTTP server started");
            break;
        case WIFI_STA:
            // Configura il modulo in modalità Station
            WiFi.disconnect(true);
            WiFi.mode(WIFI_AP_STA);
            WiFi.begin(ssid, password);
            //Serial.println("Connecting to WiFi..");
            display->clearDisplay();
            display->setCursor(0,0);
            display->println("Connecting to WiFi...");
            display->display();
            
            while (WiFi.status() != WL_CONNECTED) {
                delay(500);
                display->print(".");
                display->display();
            }
            
            if(WiFi.status() == WL_CONNECTED) {
                //Serial.println("Connected to WiFi");
                firstConnection = true;
                display->clearDisplay();
                display->setCursor(0,0);
                display->println("Connected to WiFi");                
                display->println("SSID: " + ssid);
                //!display->println("IP Address: " + WiFi.localIP());
                display->display();
            } else {
                //Serial.println("\nConnection failed, switching back to AP mode");
                display->clearDisplay();
                display->setCursor(0,0);
                display->println("Connection failed, switching back to AP mode");
                display->display();
                //configureWiFi(WIFI_AP);
            }
            break;
        default: 
            break;
    }
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

    // Avvia in modalità AP per permettere la configurazione iniziale
    configureWiFi(WIFI_AP);
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
}
