<div  align="center">

# 📬 MailTonBox 🔔
[![Made with Arduino](https://img.shields.io/badge/Made%20with-Arduino-blue.svg)](https://www.arduino.cc/)
[![ESP32](https://img.shields.io/badge/Board-ESP32-green.svg)](https://www.espressif.com/en/products/socs/esp32)
[![Platformio](https://badges.registry.platformio.org/packages/ESP32Async/library/AsyncTCP.svg)](https://platformio.org/)
[![Comm-LoRa](https://img.shields.io/badge/Comm-LoRa-orange.svg)](https://lora-alliance.org/)
[![Comm-WiFi](https://img.shields.io/badge/Comm-WiFi-lightblue.svg)](https://lora-alliance.org/)
[![Telegram Bot](https://img.shields.io/badge/Notify-Telegram-blue)](https://core.telegram.org/bots)

[![Trained with Keras](https://img.shields.io/badge/Framework-Keras-red)](https://keras.io/)
[![TensorFlow Lite](https://img.shields.io/badge/AI-TensorFlow%20Lite-lightgrey.svg)](https://www.tensorflow.org/lite)
[![TinyML](https://img.shields.io/badge/Embedded%20AI-TinyML-blueviolet)](https://www.tinyml.org/)
[![Multi-task Model](https://img.shields.io/badge/Model-Multi--Task-yellowgreen)]()

[![PWA](https://img.shields.io/badge/UI-PWA-darkgreen)](https://web.dev/progressive-web-apps/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Docs](https://img.shields.io/badge/📘_Documentation-MailTonBox-blue)](./MailTonBox.pdf)

###  Smart Mailbox IoT System
#### Internet of Things 24/25 Course Project @ UniCT DMI


<a href="https://alessandroferrante.github.io/IoT/MailTonBox" ><img src="https://github.com/AlessandroFerrante/IoT/blob/main/MailTonBox/assets/images/wallpaper.png" alt="" width="300"/></a>


MailTonBox is a distributed IoT system based on ESP32 to digitize traditional mailboxes, detecting the arrival of mail and notifying the user in real time via Telegram. 

</div>

The system includes:
- Physical detection via ultrasonic sensors
- Long-range wireless communication (LoRa)
- Automatic notifications via Telegram bot
- Environmental monitoring with embedded AI model
- Configuration interface via web app (PWA)

## 📦 System Architecture

- **MailTon**: Central node connected to the Internet via Wi-Fi, receives messages from remote nodes and manages communication with Telegram and the AI model.
- **CtrlMailBox**: Remote node inside the mailbox, detects the presence of letters and sends reports via LoRa.
- **TonyBot**: Integrated Telegram bot to receive notifications and configure the system remotely.
- **Web App**: PWA interface for configuring devices in Access Point mode.

## 🧠 Embedded Artificial Intelligence

The system implements a **multi-task** model that simultaneously executes:

- Regression: CO value estimation (ppm)
- Classification: environmental conditions (temp/humidity)
- Classification: air quality


## ⚙️ ESP32 Firmware

The ESP32 firmware is divided into two main components:

- [`MailTon/`](https://github.com/AlessandroFerrante/IoT/tree/main/MailTonBox/MailTon): code for the Central Unit
- [`CtrlMailBox/`](https://github.com/AlessandroFerrante/IoT/tree/main/MailTonBox/CtrlMailBox): code for remote knots

## 📲 Web App

- Available on GitHub Pages:
  👉 [`https://alessandroferrante.github.io/IoT/MailTonBox`](https://alessandroferrante.github.io/IoT/MailTonBox)

<a href="https://alessandroferrante.github.io/IoT/MailTonBox"><img src="https://github.com/AlessandroFerrante/IoT/blob/main/MailTonBox/assets/images/icon.png" alt="" width="200"/></a>

## 📘 Documentation

For a complete description of architecture, operation, tests and artificial intelligence embedded, consult the complete technical report:

[`📖 MailTonBox.pdf`](https://github.com/AlessandroFerrante/IoT/blob/main/MailTonBox/MailTonBox.pdf)

## 📸 Preview

![](https://github.com/AlessandroFerrante/IoT/blob/main/MailTonBox/assets/images/imgMTCMBX.jpg)
*Mailton (left) and Ctrlmailbox (right)*

## 🧰 Requirements

- Visual Studio Code with the [PlatformIO](https://platformio.org/);
- [`iot_board`](https://github.com/UniCT-Internet-of-Things/IoTBoard-Library) library provided by the course to implement the protocols using the components with Arduino Nano Esp32;

### 🛠 Libraries

| Libreria                   | Descrizione                                                 | Link                                                                                                                                          |
| -------------------------- | ----------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------- |
| `Arduino.h`              | Basic bookcase of Arduino                                 | 🔗[`Arduino Core`](https://www.arduino.cc/en/reference)                                                                                        |
| `WiFi.h`                 | Wi-Fi connection management for ESP32                | 🔗[`WiFi`](https://github.com/espressif/arduino-esp32/tree/3bfa3e0a56c80305eec90f10e8318af8d8091bab/libraries/WiFi)                            |
| `iot_board.h`            | Specific bookcase for the board used (IoTboard Unict) | 🔗[`IoTBoard Library`](https://github.com/UniCT-Internet-of-Things/IoTBoard-Library)                                                           |
| `AsyncTCP.h`             | Asinnant TCP for ESP32                                | 🔗[`AsyncTCP`](https://github.com/ESP32Async/AsyncTCP/releases)                                                                                |
| `ESPAsyncWebServer.h`    | Asynchronous web server for ESP32                           | 🔗[`ESPAsyncWebServer`](https://github.com/ESP32Async/ESPAsyncWebServer)                                                                       |
| `DNSServer.h`            | Server DNS locale per captive portal                     | 🔗[`DNSServer`](https://github.com/espressif/arduino-esp32/tree/3bfa3e0a56c80305eec90f10e8318af8d8091bab/libraries/DNSServer)                  |
| `Preferences.h`          | Non volatile memory management (NVS)                   | 🔗[`Preferences`](https://github.com/espressif/arduino-esp32/tree/3bfa3e0a56c80305eec90f10e8318af8d8091bab/libraries/Preferences)              |
| `WiFiClientSecure.h`     | HTTPS connections with certificates                           | 🔗[`WiFiClientSecure`](https://github.com/espressif/arduino-esp32/tree/3bfa3e0a56c80305eec90f10e8318af8d8091bab/libraries/NetworkClientSecure) |
| `UniversalTelegramBot.h` | Bot Telegram per ESP8266/ESP32                           | 🔗[`UniversalTelegramBot`](https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot)                                                     |
| `ArduinoJson.h`          | Parsing and generation of Json                              | 🔗[`ArduinoJson`](https://registry.platformio.org/libraries/bblanchon/ArduinoJson)                                                             |
| `DHT.h`                  | DHT11/DHT22 sensor management                  | 🔗[`DHT Sensor Library`](https://github.com/adafruit/Adafruit_Sensor)                                                                          |
| `EloquentTinyML.h`       | Execution tinyml models on microcontrolleri           | 🔗[`EloquentTinyML`](https://github.com/eloquentarduino/EloquentTinyML)                                                                        |

## 📋 License

This repository is licensed under the MIT license. See the `LICENSE` file for more details.

## 👨‍💻 Author

[Alessandro Ferrante](https://alessandroferrante.net)

📧 mailtonbox@alessandroferrante.net
