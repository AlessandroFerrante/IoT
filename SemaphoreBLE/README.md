# 🚦Semaphore with BLE Protocol

## Description

This project implements a traffic light management system using BLE (Bluetooth Low Energy). The code is split into multiple files for better organization and modularity. The `main.cpp` file implements the functions needed for the server and peripherals to work, including the `server.h` and `peripheral.h` files.

## Features

- Connection to BLE peripheral devices: The server searches for and connects to BLE traffic lights.
- Traffic light state management: Synchronizes the traffic light color between connected devices.
- Notifications and callbacks: Receives updates from peripheral traffic lights and updates the central state.
- Dynamic device management: Supports up to `MAX_SLAVES` connected peripherals.

## Project Structure

The project is split into the following files:

`server.h`: Definition of the main server functions and data structures.

`server.cpp`: Implementation of the BLE server logic.

`peripheral.h`: Definition of the main functions and data structures of the peripheral.

`peripheral.cpp`: Implementation of the BLE peripheral logic.

`main.cpp`: Entry point of the program, where the BLE server and the BLE peripheral are initialized and the main loop is started.

## FIle

1. Implementation of the BLE protocol [main.cpp](https://github.com/AlessandroFerrante/IoT/blob/main/SemaphoreBLE/main.cpp)): contains common server and peripheral implementation.
2. Implementation of the traffic light server [server.cpp](https://github.com/AlessandroFerrante/IoT/blob/main/SemaphoreBLE/server.cpp))
3. Implementation of the traffic light peripheral  [peripheral.cpp](https://github.com/AlessandroFerrante/IoT/blob/main/SemaphoreBLE/peripheral.cpp))

## Requirements

- Visual Studio Code with the [PlatformIO](https://platformio.org/);
- `iot_board` library provided by the course to implement the protocols using the components with Arduino Nano Esp32;

## License

This repository is licensed under the MIT license. See the `LICENSE` file for more details.

## Author

**Alessandro Ferrante**

Email: [github@alessandroferrante.net](mailto:github@alessandroferrante.net)
