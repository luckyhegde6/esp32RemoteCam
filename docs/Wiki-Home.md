# esp32RemoteCam Wiki

Welcome to the project wiki. This repository targets the AI-Thinker ESP32-CAM (OV2640), with stable local addressing, browser streaming and Telegram notifications/commands.

## Guides

- [ESP32-CAM setup guide](https://github.com/luckyhegde6/esp32RemoteCam/blob/feature/secure-telegram-camera/docs/ESP32-CAM-Setup.md)
- [AI-Thinker pinout and schematic](https://randomnerdtutorials.com/esp32-cam-ai-thinker-pinout/)
- [Static/fixed LAN IP](https://randomnerdtutorials.com/esp32-cam-static-fixed-ip-address-arduino/)
- [Telegram bot creation and setup](https://github.com/luckyhegde6/esp32RemoteCam/blob/feature/secure-telegram-camera/docs/Telegram-Bot-Setup.md)
- [Telegram ESP32-CAM reference tutorial](https://randomnerdtutorials.com/telegram-esp32-cam-photo-arduino/)
- [Universal Arduino Telegram Bot library](https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot)

## Diagrams

![AI-Thinker ESP32-CAM pinout](https://raw.githubusercontent.com/luckyhegde6/esp32RemoteCam/feature/secure-telegram-camera/assets/esp32-cam-ai-thinker-pinout.svg)

![Static IP setup flow](https://raw.githubusercontent.com/luckyhegde6/esp32RemoteCam/feature/secure-telegram-camera/assets/static-ip-setup-flow.svg)

![Telegram bot setup flow](https://raw.githubusercontent.com/luckyhegde6/esp32RemoteCam/feature/secure-telegram-camera/assets/telegram-bot-setup-flow.svg)



1. Confirm your board is AI-Thinker ESP32-CAM and wire it for flashing.
2. Copy `firmware/esp32cam/config.example.h` to local `config.h`; enter Wi-Fi and Telegram details.
3. Prefer router DHCP reservation for a stable local address. If using firmware static IP, configure the address, gateway, subnet and DNS for your actual LAN.
4. Compile and flash `firmware/esp32cam/esp32cam.ino`.
5. Test the printed camera address from a device on the same LAN.

## Local endpoints

When running the current firmware:

- `http://CAMERA_IP/` — camera landing page
- `http://CAMERA_IP/stream` — MJPEG stream
- `http://CAMERA_IP/capture` — single JPEG
- `http://CAMERA_IP/status` — JSON status

## Security boundary

The current HTTP camera endpoints are not authenticated and use plain HTTP. Do not port-forward the camera's web port to the public internet. For remote viewing, use a VPN server on your router if supported and connect to the camera's private LAN IP over the VPN. Telegram chat authorization protects bot commands only; it does not secure browser endpoints.

## Known gap

The current `/photo` Telegram command replies with a local `/capture` URL; direct JPEG upload into Telegram still needs to be implemented and verified.

## Images and licensing

For the AI-Thinker pinout, power pads, board schematic and Telegram flow screenshots, see the original diagrams on Random Nerd Tutorials:

- [AI-Thinker pinout and schematic figures](https://randomnerdtutorials.com/esp32-cam-ai-thinker-pinout/)
- [Static-IP illustrations](https://randomnerdtutorials.com/esp32-cam-static-fixed-ip-address-arduino/)
- [BotFather, chat-ID and Telegram demonstration images](https://randomnerdtutorials.com/telegram-esp32-cam-photo-arduino/)

Images remain on the original publisher's page unless reuse permission is verified. If you choose to copy image assets to the wiki later, preserve their attribution and license terms.

## Source code

- [Project repository](https://github.com/luckyhegde6/esp32RemoteCam)
- [Development branch](https://github.com/luckyhegde6/esp32RemoteCam/tree/feature/secure-telegram-camera)
