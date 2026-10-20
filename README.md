# esp32RemoteCam

AI-Thinker ESP32-CAM firmware with a local MJPEG stream, snapshot/status endpoints, and Telegram command polling.

## Current implementation scope

- Wi-Fi station mode and optional fixed LAN IP.
- HTTP endpoints: `/`, `/stream`, `/capture`, and `/status`.
- Telegram commands: `/start`, `/help`, `/ip`, `/status`, `/photo` (captures and uploads a JPEG directly to Telegram), `/flash_on`, and `/flash_off`.
- Telegram sender chat-ID allowlist.
- TLS certificate validation intended via a trusted root CA.

## Hardware and libraries

The initial CI compile target is pinned to **Arduino-ESP32 core 3.0.7** and uses ArduinoJson 6.21.5 to avoid depending on the latest major library release. The GitHub Actions compile workflow is a compile check only; it does not replace testing on the physical camera.

- AI-Thinker ESP32-CAM with OV2640; PSRAM recommended.
- Arduino IDE with Espressif ESP32 board package.
- Board: **AI Thinker ESP32-CAM**.
- Libraries: **UniversalTelegramBot** by Brian Lough and **ArduinoJson**, compatible with your ESP32 core version.

## Setup

1. Copy `firmware/esp32cam/config.example.h` to `firmware/esp32cam/config.h`.
2. Fill in Wi-Fi SSID/password, Telegram bot token, and numeric Telegram chat ID.
3. Use a current UniversalTelegramBot library version that includes `TelegramCertificate.h`; the sketch uses its `TELEGRAM_CERTIFICATE_ROOT` for TLS verification. Do not disable certificate validation.
4. Prefer a DHCP reservation in your router. Alternatively, uncomment `USE_STATIC_IP` and edit IP/gateway/subnet/DNS to match your network. Example addresses are placeholders.
5. Open `firmware/esp32cam/esp32cam.ino` in Arduino IDE and upload using an ESP32-CAM-MB programmer or USB-to-serial adapter. For serial upload, connect GPIO 0 to GND while flashing; disconnect GPIO 0 from GND and reset to run.
6. Open Serial Monitor at 115200 baud to see the camera IP.
7. From a device on the same LAN, open `http://CAMERA_IP/`, `http://CAMERA_IP/stream`, or `http://CAMERA_IP/status`.

## Security notes

**This initial firmware is for trusted LAN testing only.** Local HTTP camera endpoints have no user authentication or HTTPS. Do not expose port 80 to the public internet or forward it from your router. For remote live viewing without an always-on computer, the best option is a **VPN server built into your router** (for example, WireGuard or OpenVPN if your router supports it). Connect your phone/laptop to that VPN, then open the camera's private LAN IP. If the router has no VPN server, a direct VPN connection to this ESP32-CAM is not supported by the normal Arduino firmware; you will need a gateway device or a separately designed authenticated relay. Do not forward the camera's HTTP port to the public internet. Telegram chat authorization protects bot commands only; it does not protect the web server.

The `/photo` command uploads a captured JPEG using the library's binary-photo API. TLS uses the library-provided `TELEGRAM_CERTIFICATE_ROOT` and synchronizes time with NTP before Telegram requests.

GPIO 4 drives the onboard flash LED and is also associated with the microSD interface. Do not use the flash and microSD simultaneously without checking the board wiring.

## Roadmap

- Upload captured JPEG frames directly to Telegram.
- Add local web authentication or an authenticated gateway.
- Improve reconnection and error handling.
- Pin library/core versions and compile/test against a real board.
