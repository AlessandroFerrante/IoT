
# ZigBee

ZigBee is a low-power wireless communication protocol designed for home automation and Internet of Things (IoT) applications. It is a low-power technology that enables the creation of networks of sensors and devices that communicate with each other in a secure and reliable way. ZigBee is based on the IEEE 802.15.4 standard and supports both point-to-point and mesh network communications.

The main features of ZigBee are:

* **Low power consumption** : allows battery-powered devices to operate for years.
* **Reliability** : supports a mesh network that increases the robustness of communication.
* **Security** : includes advanced encryption features to ensure data protection.
* **Compatibility** : is used in a variety of devices, from sensors to lights to smart thermostats.

## ZigBee Implementation

This project implements a ZigBee device using the `iot_board` library. It shows how a device can communicate with another ZigBee device to send and receive messages. It is designed for automation applications, such as controlling smart devices via the ZigBee network.

## Hardware Used

* ZigBee compatible device (e.g. ESP32 with ZigBee module)
* OLED display (for displaying messages and statistics)
* Buttons (for optional hardware interactions)
* LEDs (for visual feedback)

## Operation

The project consists of two main components:

1. **ZigBee Device** :

* It manages communication with other ZigBee devices.
* It can receive and send messages, communicating via the ZigBee network.
* It displays information such as the received message, the address of the sending device and other statistics.

1. **Control via Buttons** :

* Buttons are used to activate and deactivate devices or perform other actions, interactively.

## Message Structure

Each ZigBee message sent contains the following fields:

* **Destination Address** (2 bytes)
* **Sender Address** (2 bytes)
* **Message ID** (1 byte)
* **Payload Length** (1 byte)
* **Payload Data** (text string, e.g. commands to turn devices on or off)

## Address Configuration

* The destination device has an address `destination = 0xFF`.
* Each device has its own `localAddress`, which identifies it in the ZigBee network.

## Installation and Use

1. Upload the code to the ZigBee device.
2. Connect the device to the OLED display and the buttons for hardware interaction.
3. Power on the device and observe the messages sent or received on the serial or on the OLED display.
4. Interact with the device using the buttons.

## Customizations

* Change the value of `destination` to send messages to a specific device.
* Add new commands to control other devices in the ZigBee network.
* Implement network resource management to improve communication reliability.

## Requirements

- Visual Studio Code with the [PlatformIO](https://platformio.org/);
- `iot_board` library provided by the course to implement the protocols using the components with Arduino Nano Esp32;

## License

This repository is licensed under the MIT license. See the `LICENSE` file for more details.

## Author

**Alessandro Ferrante**

Email: [github@alessandroferrante.net](mailto:github@alessandroferrante.net)
