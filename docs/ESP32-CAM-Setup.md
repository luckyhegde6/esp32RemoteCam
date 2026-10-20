# esp32RemoteCam: ESP32-CAM Setup Guide

This guide documents the AI-Thinker ESP32-CAM + OV2640 setup used by this project, router-local fixed IP configuration, and the safe way to access the camera over a LAN or VPN.

## References and attribution

This guide is an original project-specific summary. Consult the original tutorials for their full explanations and images:

- [Random Nerd Tutorials — AI-Thinker ESP32-CAM pinout](https://randomnerdtutorials.com/esp32-cam-ai-thinker-pinout/)
- [Random Nerd Tutorials — ESP32-CAM static/fixed IP](https://randomnerdtutorials.com/esp32-cam-static-fixed-ip-address-arduino/)
- [Random Nerd Tutorials — ESP32-CAM video streaming](https://randomnerdtutorials.com/esp32-cam-video-streaming-face-recognition-arduino-ide/)
- [Random Nerd Tutorials — Telegram photos](https://randomnerdtutorials.com/telegram-esp32-cam-photo-arduino/)
- [Universal Arduino Telegram Bot](https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot)

The referenced diagrams and screenshots remain the property of their respective authors. Rather than copying third-party image files into this repository without confirming reuse rights, the pinout and schematic are linked to the original page below. Open that page for the original diagrams.

## 1. Hardware

Recommended baseline:

- AI-Thinker ESP32-CAM with OV2640 camera module and PSRAM
- ESP32-CAM-MB USB programmer or a USB-to-serial/FTDI adapter
- Stable regulated 5 V supply capable of handling camera/Wi-Fi current peaks
- 2.4 GHz Wi-Fi network
- Optional microSD card (not required for current web streaming)

### Power and serial upload

The AI-Thinker board does not include a full USB programmer on the camera board itself.

For a USB-to-serial adapter:

| ESP32-CAM | USB-to-serial adapter |
|---|---|
| 5V | regulated 5V |
| GND | GND |
| U0R / GPIO3 (RX) | TX |
| U0T / GPIO1 (TX) | RX |
| GPIO0 | GND only during flashing |

Use a suitable 5 V supply and common ground. Do not connect the adapter's 5 V and 3.3 V supply outputs simultaneously. Check the silkscreen labels on your adapter rather than relying on pin order.

To upload: connect GPIO0 to GND, reset/power-cycle, upload, then disconnect GPIO0 from GND and reset again to run normally.

## 2. AI-Thinker pinout and camera wiring

**Original pinout diagram and schematic:** [Random Nerd Tutorials — ESP32-CAM AI-Thinker pinout](https://randomnerdtutorials.com/esp32-cam-ai-thinker-pinout/). The article provides the pinout figure, power-pin details, camera mapping, SD interface mapping, and a schematic source.

The OV2640 camera GPIO mapping used in this repository is:

| Camera signal | ESP32 GPIO | Project camera field |
|---|---:|---|
| PWDN | 32 | `pin_pwdn` |
| RESET | -1 (not connected) | `pin_reset` |
| XCLK | 0 | `pin_xclk` |
| SCCB SDA / SIOD | 26 | `pin_sccb_sda` |
| SCCB SCL / SIOC | 27 | `pin_sccb_scl` |
| D0 / Y2 | 5 | `pin_d0` |
| D1 / Y3 | 18 | `pin_d1` |
| D2 / Y4 | 19 | `pin_d2` |
| D3 / Y5 | 21 | `pin_d3` |
| D4 / Y6 | 36 | `pin_d4` |
| D5 / Y7 | 39 | `pin_d5` |
| D6 / Y8 | 34 | `pin_d6` |
| D7 / Y9 | 35 | `pin_d7` |
| VSYNC | 25 | `pin_vsync` |
| HREF | 23 | `pin_href` |
| PCLK | 22 | `pin_pclk` |

These pins are for the **AI-Thinker ESP32-CAM**. Other camera boards can use different mappings.

### Important pin caveats

- GPIO0 is the camera XCLK pin and also controls bootloader mode. Ground it only for flashing; remove the jumper before normal operation.
- GPIO1/GPIO3 are UART TX/RX and are used for programming/log output.
- GPIO4 drives the white flash LED and is also tied to the microSD interface; avoid using flash and SD concurrently unless you have confirmed the mode and board wiring.
- GPIO12, GPIO13, GPIO14, GPIO15, GPIO2, and GPIO4 are used by the microSD interface in the standard 4-bit configuration.
- GPIO16 is commonly associated with PSRAM; do not assume it is free while camera/PSRAM is enabled.
- The VCC pad on some board revisions is an output rail selector/output, not a substitute for the marked 5V input.

## 3. Arduino environment

1. Install Arduino IDE.
2. Install Espressif's ESP32 board package.
3. Select **AI Thinker ESP32-CAM**.
4. Install the required libraries:
   - UniversalTelegramBot by Brian Lough
   - ArduinoJson 6.x for the currently pinned CI setup
5. Use the project's compile workflow and README for the currently selected core version. A successful host CI compile is not a substitute for testing with the actual board.

## 4. Configure Wi-Fi and credentials

From the project root/branch, copy:

```text
firmware/esp32cam/config.example.h
```

to:

```text
firmware/esp32cam/config.h
```

Enter the Wi-Fi SSID/password, Telegram bot token, and numeric authorized Telegram chat ID in `config.h`. This file is ignored by Git. Do not commit secrets or paste the bot token into issues/screenshots.

Open `firmware/esp32cam/esp32cam.ino` in Arduino IDE, compile, flash, then open Serial Monitor at 115200 baud.

## 5. Static/fixed LAN IP

The project's sketch supports `USE_STATIC_IP`, based on the `WiFi.config()` approach documented by Random Nerd Tutorials.

**Recommended option: DHCP reservation.** In the router's DHCP/Address Reservation page, reserve the same private IP for the ESP32-CAM's Wi-Fi MAC address. This normally provides stable addressing while keeping network configuration centralized.

**Alternative: static IP in firmware.**

1. Check your laptop/phone's local IP address and default gateway. Example: a device at `192.168.1.25` with gateway `192.168.1.1` is probably on a `192.168.1.x` LAN.
2. In router settings, choose an unused LAN IP for the camera; ensure it does not collide with another device or DHCP lease. Ideally keep it outside the dynamic DHCP pool or create a reservation.
3. Open `firmware/esp32cam/config.example.h`, copy it to `config.h`, uncomment `#define USE_STATIC_IP`, then update the address octets:
   - `STATIC_IP_1..4`: camera IP
   - `GATEWAY_1..4`: router LAN IP
   - `SUBNET_1..4`: router subnet mask (commonly `255.255.255.0`, but verify)
   - `DNS1_1..4` and `DNS2_1..4`: valid DNS servers
4. Make sure the camera address and gateway are on the same subnet. Do not blindly use the sample `192.168.1.x` addresses: many home routers use a different subnet such as `192.168.0.x` or `192.168.29.x`.
5. Flash and reboot. Check Serial Monitor for the resulting IP. From a device on the same LAN, visit `http://CAMERA_IP/`.

Sketch configuration shape:

```cpp
WiFi.mode(WIFI_STA);
#ifdef USE_STATIC_IP
IPAddress localIp(192, 168, 1, 60);  // choose unused IP for your LAN
IPAddress gateway(192, 168, 1, 1);   // your router LAN IP
IPAddress subnet(255, 255, 255, 0);  // use your actual subnet mask
IPAddress dns1(1, 1, 1, 1);
IPAddress dns2(8, 8, 8, 8);
WiFi.config(localIp, gateway, subnet, dns1, dns2);
#endif
WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
```

This is an explanatory example only; in this repository, the IP octets are supplied through `config.h`.

## 6. Available local endpoints

With the current firmware, once the camera is connected to Wi-Fi:

| URL | Purpose |
|---|---|
| `http://CAMERA_IP/` | landing page |
| `http://CAMERA_IP/stream` | MJPEG live stream |
| `http://CAMERA_IP/capture` | current JPEG snapshot |
| `http://CAMERA_IP/status` | JSON status |

Use the actual IP printed in Serial Monitor or returned by `/ip` in Telegram.

## 7. Remote access and security

A static/private LAN IP does **not** make the ESP32-CAM reachable from the public internet. It only makes the camera's address stable within the local network.

The current web endpoints use plain HTTP and do not implement web authentication. Do not forward port 80 to the internet.

Preferred remote live-view architecture when there is no always-on gateway:

1. Check whether your router supports a VPN server such as WireGuard or OpenVPN.
2. Configure the VPN on the router and a client on your phone/laptop.
3. Connect the client to the VPN while away from home.
4. Open the camera's private LAN address over the VPN.

If the router does not provide a suitable VPN server, a VPN-capable router firmware, gateway, or authenticated relay is needed. A normal Arduino ESP32-CAM sketch does not simply run a full Tailscale client. Also, an ISP public IP may be unavailable for inbound connections under CGNAT.

Telegram works remotely for bot messages/photos when the ESP32-CAM has outbound internet access, independently of direct remote browser viewing. Telegram sender authorization only protects bot commands; it does not protect the local HTTP endpoints.

## 8. Troubleshooting

- **Camera init fails:** confirm AI-Thinker pin mapping, camera ribbon orientation, adequate power, and PSRAM settings.
- **Brownout/restarts:** use a stable 5 V supply with adequate current capability and short wires.
- **Wi-Fi connects only with DHCP:** check gateway, subnet, IP conflicts, and static-IP configuration order. Call `WiFi.config()` before `WiFi.begin()`.
- **Unable to upload:** GPIO0 must be connected to GND during flashing; remove it and reset afterward.
- **Telegram TLS errors:** ensure valid system time/NTP is available and the Telegram root CA configured by the library is current. Never fix certificate problems by disabling TLS verification.
- **Browser stream unavailable remotely:** first verify it on the LAN. Then use VPN routing; do not expose the unauthenticated HTTP endpoint directly.

## References

- [AI-Thinker pinout and schematic](https://randomnerdtutorials.com/esp32-cam-ai-thinker-pinout/)
- [Static/fixed IP setup](https://randomnerdtutorials.com/esp32-cam-static-fixed-ip-address-arduino/)
- [Streaming tutorial](https://randomnerdtutorials.com/esp32-cam-video-streaming-face-recognition-arduino-ide/)
- [Telegram camera tutorial](https://randomnerdtutorials.com/telegram-esp32-cam-photo-arduino/)
- [Universal Arduino Telegram Bot library](https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot)
