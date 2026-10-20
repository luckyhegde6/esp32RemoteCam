# esp32RemoteCam

AI-Thinker ESP32-CAM firmware with a local MJPEG stream, snapshot/status endpoints, and Telegram command polling.

## Documentation

- [ESP32-CAM Setup Guide](docs/ESP32-CAM-Setup.md) — hardware, pinout reference, flashing, fixed LAN IP, endpoints and remote access security.
- [Telegram Bot Setup](docs/Telegram-Bot-Setup.md) — BotFather, user/chat ID, secrets, dependencies, commands and test steps.
- [GitHub Wiki home](https://github.com/luckyhegde6/esp32RemoteCam/wiki) — online wiki landing page.

Reference sources: [AI-Thinker pinout](https://randomnerdtutorials.com/esp32-cam-ai-thinker-pinout/), [static/fixed IP](https://randomnerdtutorials.com/esp32-cam-static-fixed-ip-address-arduino/), and [Telegram photo bot](https://randomnerdtutorials.com/telegram-esp32-cam-photo-arduino/). Third-party diagrams and screenshots are linked at their original pages rather than copied without confirming reuse rights.

## Current implementation scope

- Wi-Fi station mode and optional fixed LAN IP.
- HTTP endpoints: `/`, `/stream`, `/capture`, and `/status`.
- Telegram commands: `/start`, `/help`, `/ip`, `/status`, `/photo` (currently replies with a local snapshot URL), `/flash_on`, and `/flash_off`.
- Telegram sender chat-ID allowlist.
- TLS certificate validation intended via a trusted root CA.

## Hardware and libraries

The initial CI compile target is pinned to **Arduino-ESP32 core 3.0.7** and uses ArduinoJson 6.21.5. CI is a compile check only; it does not replace testing on the physical camera.

- AI-Thinker ESP32-CAM with OV2640; PSRAM recommended.
- Arduino IDE with Espressif ESP32 board package; board: **AI Thinker ESP32-CAM**.
- Libraries: UniversalTelegramBot by Brian Lough and ArduinoJson, compatible with the chosen core version.

## Setup

1. Copy `firmware/esp32cam/config.example.h` to `firmware/esp32cam/config.h`.
2. Fill in Wi-Fi SSID/password, Telegram bot token, and numeric Telegram chat ID.
3. Use a current UniversalTelegramBot library version with `TelegramCertificate.h`; do not disable TLS certificate validation.
4. Prefer a DHCP reservation in your router. Alternatively, uncomment `USE_STATIC_IP` and edit IP/gateway/subnet/DNS in the local config to match your network.
5. Open `firmware/esp32cam/esp32cam.ino` in Arduino IDE and upload using an ESP32-CAM-MB programmer or USB-to-serial adapter. For serial upload, connect GPIO 0 to GND while flashing; disconnect GPIO 0 from GND and reset to run.
6. Open Serial Monitor at 115200 baud to see the camera IP.
7. From a device on the same LAN, open `http://CAMERA_IP/`, `http://CAMERA_IP/stream`, or `http://CAMERA_IP/status`.

## Security notes

**This initial firmware is for trusted LAN testing only.** Local HTTP camera endpoints have no user authentication or HTTPS. Do not expose port 80 to the public internet or forward it from your router. For remote live viewing without an always-on computer, the best option is a VPN server built into your router (for example, WireGuard or OpenVPN if supported). Connect your phone/laptop to that VPN, then open the camera's private LAN IP. If the router has no VPN server, a VPN-capable router, gateway device, or authenticated relay is needed. Telegram chat authorization protects bot commands only; it does not protect the web server.

**Current `/photo` behavior:** the Telegram command currently returns a URL for the local `/capture` endpoint and does not attach a JPEG to Telegram yet. Direct photo upload remains a follow-up implementation task. TLS uses the library-provided root CA and requires correct time for certificate validation.

GPIO 4 drives the onboard flash LED and is also associated with the microSD interface. Do not use the flash and microSD simultaneously without checking the board wiring.

## Roadmap

- Implement and test JPEG upload directly into Telegram for `/photo`.
- Add local web authentication or an authenticated gateway.
- Improve reconnection and error handling.
- Pin library/core versions and compile/test against a real board.
