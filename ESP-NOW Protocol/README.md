# ESP-NOW Protocol

## Description
ESP-NOW is a protocol developed by Espressif, which enables multiple devices to communicate with one another without using Wi-Fi. The protocol is similar to the low-power 2.4GHz wireless connectivity. The pairing between devices is needed prior to their communication. After the pairing is done, the connection is safe and peer-to-peer, with no handshake being required.

This means that after pairing a device with each other, the connection is persistent. In other words, if suddenly one of your boards loses power or resets, when it restarts, it will automatically connect to its peer to continue the communication.

### ESP-NOW supports:
Encrypted and unencrypted unicast communication;
Mixed encrypted and unencrypted peer devices;
Up to 250-byte payload can be carried;
Sending callback function that can be set to inform the application layer of transmission success or failure.

### ESP-NOW limitations:
Limited encrypted peers. 10 encrypted peers at the most are supported in Station mode; 6 at the most in SoftAP or SoftAP + Station mode;
Multiple unencrypted peers are supported, however, their total number should be less than 20, including encrypted peers;
Payload is limited to 250 bytes.
In simple words, ESP-NOW is a fast communication protocol that can be used to exchange small messages (up to 250 bytes) between ESP32 boards.

To communicate via ESP-NOW, you need to know the MAC Address of the ESP32 receiver.

## FIle 
1. Implementation of the protocol [espnow.cpp]((https://github.com/AlessandroFerrante/Embedded-Systems/blob/main/ESP-NOW%20Protocol/espnow.cpp))
2. Example of the application of the protocol [semaphoreESPNOW.cpp]((https://github.com/AlessandroFerrante/Embedded-Systems/blob/main/ESP-NOW%20Protocol/semaphoreESPNOW.cpp))

## Requirements
- Visual Studio Code with the [PlatformIO](https://platformio.org/);
- `iot_board` library provided by the course to implement the protocols using the components with Arduino Nano Esp32;

## License
This repository is licensed under the MIT license. See the `LICENSE` file for more details.

## Author
**Alessandro Ferrante**