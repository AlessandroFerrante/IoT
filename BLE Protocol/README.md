# BLE Protocol

Bluetooth Low Energy (BLE) is a wireless communication protocol designed for short-range communication with low power consumption. It is widely used in various applications such as wearable devices, smart home devices, and health monitoring systems. BLE operates in the 2.4 GHz ISM band and supports data rates up to 2 Mbps.

The BLE protocol is structured around two main types of devices: peripheral and central. Peripheral devices are typically small, low-power devices that advertise their presence and wait for a connection from a central device. Central devices, on the other hand, are usually more powerful and initiate connections with peripheral devices to exchange data.

In this repository, you will find implementations for both peripheral and central devices using the Arduino Nano ESP32. The code is organized to facilitate the development and testing of BLE applications, ensuring a clear separation between the different roles and functionalities.

## File Structure
The files in this repository are organized to separate the implementations of peripheral and central devices for the BLE protocol. Below is a list of the main files and directories:

- [`peripheral`](https://github.com/AlessandroFerrante/Embedded-Systems/blob/main/BLE%20Protocol/peripheral.cpp): Contains the implementation for the BLE peripheral devices.
- [`server`](https://github.com/AlessandroFerrante/Embedded-Systems/blob/main/BLE%20Protocol/server.cpp): Contains the implementation for the BLE server devices.
- [`main`](https://github.com/AlessandroFerrante/Embedded-Systems/blob/main/BLE%20Protocol/main.cpp): Contains the main logic and common functionalities shared between peripheral and central devices.

Each file includes the necessary code and configurations to implement the BLE protocol using the Arduino Nano ESP32.

## Requirements
- Visual Studio Code with the [PlatformIO](https://platformio.org/);
- `iot_board` library provided by the course to implement the protocols using the components with Arduino Nano Esp32;

## License
This repository is licensed under the MIT license. See the `LICENSE` file for more details.

## Author
**Alessandro Ferrante**