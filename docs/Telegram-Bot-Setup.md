# Telegram Bot Setup for esp32RemoteCam

This guide creates a Telegram bot and configures it to communicate with the ESP32-CAM. It is a project-specific summary of the workflow described by [Random Nerd Tutorials — Telegram ESP32-CAM Take and Send Photo](https://randomnerdtutorials.com/telegram-esp32-cam-photo-arduino/).

## 1. Create a bot with BotFather

1. Open Telegram and start a conversation with the verified BotFather account: [t.me/BotFather](https://t.me/BotFather).
2. Send `/newbot`.
3. Choose a display name, then a unique username that ends in `bot`.
4. BotFather returns a bot token. Treat it like a password; anyone possessing it can control the bot. Store it only in your local ignored `firmware/esp32cam/config.h`.
5. Open your newly created bot using the URL BotFather provides and press **Start** (or send `/start`). Bots generally cannot initiate a private conversation until the user has started them.

If the token is exposed, revoke/regenerate it through BotFather immediately and update the local configuration.

## 2. Find your Telegram user/chat ID

Use a Telegram ID helper such as [@myidbot](https://t.me/myidbot) and send `/getid`, or use another trusted method to retrieve your numeric user ID. Review the response carefully.

For this project, `TELEGRAM_CHAT_ID` is an allowlist entry used to decide which incoming chat can trigger commands. For a private chat, the numeric chat ID is typically your user chat ID. Groups have different chat IDs; do not substitute a group ID unless group operation is intentionally configured.

## 3. Configure the project

Copy:

```sh
cp firmware/esp32cam/config.example.h firmware/esp32cam/config.h
```

On Windows, use File Explorer or:

```powershell
Copy-Item firmware/esp32cam/config.example.h firmware/esp32cam/config.h
```

Edit `firmware/esp32cam/config.h` locally:

```cpp
#define WIFI_SSID "YOUR_2_4_GHZ_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#define TELEGRAM_BOT_TOKEN "PASTE_BOTFATHER_TOKEN_HERE"
#define TELEGRAM_CHAT_ID "YOUR_NUMERIC_CHAT_ID"
```

Do not commit `config.h`, share the token in screenshots, or put secrets in a public wiki. The repository ignores `config.h`; verify this before committing.

## 4. Install dependencies

The firmware uses:

- [Universal Arduino Telegram Bot](https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot) by Brian Lough
- [ArduinoJson](https://arduinojson.org/) 6.x for the currently pinned compile setup
- Espressif ESP32 Arduino core
- Built-in Arduino ESP32 libraries `WiFi`, `WiFiClientSecure`, and `esp_camera`

Follow the project's README and CI workflow for its current core/library versions. UniversalTelegramBot upstream instructions may change; use a compatible release containing the required certificate header/API.

## 5. TLS and clock requirements

Telegram's Bot API must be contacted over HTTPS. The project uses `WiFiClientSecure` and a trusted root certificate supplied by the Telegram bot library. Certificate validation requires a plausible system clock, so NTP synchronization must succeed before Telegram HTTPS requests are sent.

Do **not** use `setInsecure()` as a workaround. If TLS fails, inspect serial logs, Wi-Fi/internet connectivity, NTP access, CA certificate validity, and library versions.

## 6. Current project command behavior

The current branch processes the following commands from the configured authorized chat ID:

| Command | Current behavior |
|---|---|
| `/start` | shows help |
| `/help` | shows help |
| `/ip` | returns camera LAN URL and stream URL |
| `/status` | reports online state, LAN IP, Wi-Fi RSSI, uptime and free heap |
| `/photo` | currently replies with a local `/capture` URL; it does **not** yet upload a JPEG image |
| `/flash_on` | turns GPIO4 flash LED on |
| `/flash_off` | turns GPIO4 flash LED off |

The original Random Nerd Tutorials example supports sending a captured photo through Telegram, but the current project branch's `/photo` handler only sends a local URL. This is a known gap between intended and current behavior; do not assume an image attachment will arrive until the firmware is updated and tested.

The IP and snapshot URLs are private LAN URLs. They normally will not open directly from outside the home network unless you are connected to the home network or an appropriately configured VPN.

## 7. Test sequence

1. Flash the ESP32-CAM with `config.h` containing valid credentials.
2. Open Serial Monitor at 115200 baud and wait for Wi-Fi connection and the printed camera IP.
3. Open the bot and send `/start`; verify the response.
4. Send `/status` and `/ip`; confirm the IP matches the Serial Monitor.
5. From a device on the same Wi-Fi, open `http://CAMERA_IP/` and `http://CAMERA_IP/stream`.
6. Send `/flash_on` then `/flash_off`; verify the onboard LED changes as expected.
7. Send a command from a different Telegram account; it should be ignored because that sender is not allowlisted.
8. Test `/photo`; currently expect the bot to return a local snapshot URL, not a photo attachment.

## 8. Security and remote access

- Keep the bot token secret; rotate it immediately if leaked.
- Keep `TELEGRAM_CHAT_ID` restricted to your intended user.
- Telegram authorization and web-server authorization are separate. The current local HTTP endpoints have no authentication/TLS.
- Do not port-forward the camera's HTTP port to the public internet.
- For remote live streaming, prefer a VPN server on a compatible home router. Connect your phone to the VPN, then open the camera's private IP. If the router does not support VPN server mode, use a gateway or an authenticated relay.
- Telegram photo delivery and web live streaming are separate features: bot commands can work over outbound internet while the browser stream remains LAN/VPN-only.

## References

- [Random Nerd Tutorials — Telegram ESP32-CAM photo](https://randomnerdtutorials.com/telegram-esp32-cam-photo-arduino/)
- [Universal Arduino Telegram Bot source and usage](https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot)
- [Telegram BotFather](https://t.me/BotFather)
- [Telegram Bot API](https://core.telegram.org/bots/api)
