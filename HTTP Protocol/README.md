# HTTP Protocol Server & Client

This repository contains two programs for ESP32:

1. **HTTP Server**: A web server that handles GET and POST requests.
2. **HTTP Client**: A client that connects to an external API and displays data on a display.

## 1. HTTP Server

### Requirements

* ESP Async WebServer
* AsyncTCP
* iot_board (for display and LED management)

### Features

* Exposes an HTTP server on port 80.
* Handles GET requests to the root (`/`) returning "Hello, world".
* Handles GET requests with parameters on `/get`, returning the received value.
* Handles POST requests on `/post` with parameters in the request body.
* Shows the WiFi connection status on the OLED display.

### Installation

1. Clone the repository and upload the code to an ESP32.
2. Install the necessary libraries via Arduino IDE or PlatformIO.
3. Configure WiFi credentials in the code.

### Usage

Once booted, the device will connect to the WiFi network and start serving HTTP requests. The code uses the ESP Async WebServer and AsyncTCP libraries to handle asynchronous requests, as well as IoTBoard.

Using this syntax with `curl` is an alternative to other HTTP request methods

```
curl "http://<ESP32_IP>/get?parameter=test"
```

Example POST request:

```
curl -X POST "http://<ESP32_IP>/post" -d "parameter=test"
```

## 2. HTTP Client

### Requirements

* WiFiClientSecure
* HTTPClient
* ArduinoJson
* iot_board

### Features

* Connects to a WiFi network.
* Retrieves Bitcoin price data from the CoinDesk API.
* Displays the updated value on an OLED display.
* Refreshes the data every 2 seconds.

## FIle

1. Implementation of the client HTTP [http_client.cpp](https://github.com/AlessandroFerrante/IoT/blob/main/http/http_client.cpp)
2. Implementation of the server HTTP protocol [http_server.cpp](https://github.com/AlessandroFerrante/IoT/blob/main/http/http_server.cpp)
3. Implementation of the server HTTP protocol with filesystems [http_server_filesystem.cpp](https://github.com/AlessandroFerrante/IoT/blob/main/http/http_server_filesystem.cpp)

## Requirements

- Visual Studio Code with the [PlatformIO](https://platformio.org/);
- `iot_board` library provided by the course to implement the protocols using the components with Arduino Nano Esp32;

## License

This repository is licensed under the MIT license. See the `LICENSE` file for more details.

## Author

**Alessandro Ferrante**

Email: [github@alessandroferrante.net](mailto:github@alessandroferrante.net)
