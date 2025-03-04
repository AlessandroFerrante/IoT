# MQTT

This repository contains an MQTT client for ESP32 that connects to a public broker and publishes/subscribes messages.

## Features

- Connect to WiFi.
- Connect to an MQTT broker at `broker.hivemq.com` with port `8883` (MQTT over TLS).
- Publish a message to `test/status` every 4 seconds.
- Subscribe to the topic `test/recv1` to receive messages.
- Show the connection status on the OLED display.

Once booted, the device will connect to WiFi and the MQTT broker. If the connection is successful:

- Every 4 seconds it will publish a message to `test/status`.
- Listen on `test/recv1` to receive messages.

## Notes

- Make sure your MQTT broker supports secure connections.
- If necessary, modify `mqtt_server` and `mqtt_port` in the code.
- To test the communication, you can use tools like MQTT Explorer or `mosquitto_sub`/`mosquitto_pub`.

## Requirements

- Visual Studio Code with the [PlatformIO](https://platformio.org/);
- `iot_board` library provided by the course to implement the protocols using the components with Arduino Nano Esp32;

## License

This repository is licensed under the MIT license. See the `LICENSE` file for more details.

## Author

**Alessandro Ferrante**

Email: [github@alessandroferrante.net](mailto:github@alessandroferrante.net)
