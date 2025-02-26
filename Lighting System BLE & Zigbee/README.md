# 💡 Smart IoT Lighting System

## Description
This project implements a smart lighting system based on BLE (Bluetooth Low Energy) and ZigBee technologies. The system includes three main components:

1. **BLE-ZigBee Gateway**: Works as a ZigBee coordinator and BLE server for managing communications between devices.
2. **BLE Button**: A BLE client device that allows you to control the lamp remotely.
3. **ZigBee Lamp**: A device that receives commands from the gateway and adjusts its state based on the commands received.

## Code Structure
The project is divided into the following files:

- `main.cpp`: The main file that manages the initialization and loop of the various devices.
- `gateway.cpp`: Contains the code for the operation of the BLE-ZigBee gateway.
- `BLEbtn.cpp`: Defines the behavior of the BLE button.
- `lamp.cpp`: Contains the logic for managing the ZigBee lamp.
- `gateway.h`,` blebtn.h`, `lamp.h`: They contain the declarations necessary for the functioning of the gateway, the BLE button and the zigbee lamp. These files are included in the respective `.cpp` to modularize the code and separate the implementation from the definition of the interfaces.

## Operation
- The **BLE button** searches for the gateway and connects to it.
- The **BLE-ZigBee gateway** receives commands from the button and forwards them to the lamp via ZigBee.
- The **ZigBee lamp** turns the light on/off or changes color based on the commands received.

## Hardware Requirements
- A microcontroller with BLE support for the gateway and the button (e.g. ESP32).
- A ZigBee module for the gateway and the lamp.
- An OLED display for debugging and viewing the system status.
- Physical buttons for manually controlling the lamp.

## Configuration and Compilation
1. Install the required libraries for BLE and ZigBee.
2. Select the device to compile by changing the following lines in `main.cpp`:
```cpp
#define GATEWAY // To compile the gateway code
//#define BLE_BTN // To compile the BLE button code
//#define ZGB_LAMP // To compile the lamp code
```
3. Compile and upload the code to the respective microcontroller.

## Lamp Control
- By pressing the BLE button, the gateway receives the signal and forwards it to the lamp.
- The lamp changes state or color based on the commands received.
## Requirements

- Visual Studio Code with the [PlatformIO](https://platformio.org/);
- `iot_board` library provided by the course to implement the protocols using the components with Arduino Nano Esp32;

## License
This repository is licensed under the MIT license. See the `LICENSE` file for more details.

## Author
**Alessandro Ferrante**

Email: [github@alessandroferrante.net](mailto:github@alessandroferrante.net)
