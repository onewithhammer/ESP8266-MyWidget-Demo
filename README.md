# ESP8266-MyWidget-Demo

This project can be used as a framework, learning tool or as a template for starting a new ESP8266 project. It contains many UI and API components to build an ESP8266 project.

## Features

- ESP8266 Development
- mDNS - multicast DNS Support (mywidget.local)
- Async Web Server (ESPAsyncWebServer)
- Async Web Services (ESPAsyncWebServer)
- Async MQTT (PangolinMQTT)
- LittleFS File System
- Embedded Bootstrap (5.3.8)
- Embedded jQuery (4.0.0)
- Embedded HTML, CSS, JS and Images
- Save/Load Config File (text)
- OTA (Over the Air) Updates
- HTTP GET/POST Method w TEXT/JSON response
- API Web Services
- Dynamically Updating Web Site (Web Services using JS)
- Multiple Interrupt Timers (ESP8266TimerInterrupt)
- Flash Onboard LED Using Interrupt Timer

## Hardware

- WeMos D1 Mini (ESP8266)

## Version

2.0.0

## Dependencies

| Library | Version |
|---|---|
| [ESPAsyncTCP](https://github.com/me-no-dev/ESPAsyncTCP) | latest |
| [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer) | ^1.2.3 |
| [PangolinMQTT](https://github.com/philbowles/PangolinMQTT) | v1.0.0 |
| [ESP8266TimerInterrupt](https://github.com/khoih-prog/ESP8266TimerInterrupt) | latest |
| [Bootstrap](https://getbootstrap.com/) | 5.3.8 |
| [jQuery](https://jquery.com/) | 4.0.0 |

## Getting Started

### Prerequisites

- [PlatformIO](https://platformio.org/) (CLI or IDE plugin)
- An MQTT broker (optional, for MQTT features)

### Configuration

1. Create a `src/secrets.h` file with your WiFi and MQTT credentials:

```cpp
#define WIFI_SSID "your-ssid"
#define WIFI_PASS "your-password"
#define MQTT_HOST "your-mqtt-broker-ip"
```

### Build and Upload

```bash
# Build firmware
pio run

# Upload firmware to board
pio run --target upload

# Build and upload LittleFS filesystem
pio run --target buildfs
pio run --target uploadfs
```

### Serial Monitor

```bash
pio device monitor -b 115200
```

## Usage

Once uploaded, open [http://mywidget.local](http://mywidget.local) in your browser.

### Web Pages

- **Home** (`index.html`) — Feature list and project info
- **Config** (`config.html`) — Device configuration, LED toggle, interrupt speed control, and uptime display

### API Endpoints

| Endpoint | Method | Response | Description |
|---|---|---|---|
| `/heap` | GET | TEXT | Free heap memory |
| `/intcount` | GET | TEXT | Interrupt counters (params: `1`, `2`, `3`) |
| `/intcount2` | GET | TEXT | Interrupt counter 2 |
| `/intcount3` | GET | TEXT | Interrupt counter 3 |
| `/status` | GET | TEXT | Device status (params: `network`, `signal`, `chipInfo`, `heap`, `fs`) |
| `/status-json` | GET | JSON | Device status (same params as above) |
| `/counter` | POST | TEXT | Set MQTT publish counter |

### WebSocket

WebSocket endpoint at `/ws` supports real-time communication for:
- LED toggle
- Configuration read/write
- Interrupt speed control
- Uptime updates

## License

MIT License - Copyright (c) 2021-2026 Tony Keith
