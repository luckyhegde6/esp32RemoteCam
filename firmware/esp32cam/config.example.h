#pragma once
// Copy this file to config.h and fill in your own values.
// NEVER commit config.h or share your Telegram bot token.
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#define TELEGRAM_BOT_TOKEN "123456789:REPLACE_WITH_BOT_TOKEN"
#define TELEGRAM_CHAT_ID "REPLACE_WITH_NUMERIC_CHAT_ID"

// Prefer a router DHCP reservation. Enable USE_STATIC_IP only after confirming
// your subnet and choosing an unused IP outside the router's DHCP pool.
// #define USE_STATIC_IP
#define STATIC_IP_1 192
#define STATIC_IP_2 168
#define STATIC_IP_3 1
#define STATIC_IP_4 60
#define GATEWAY_1 192
#define GATEWAY_2 168
#define GATEWAY_3 1
#define GATEWAY_4 1
#define SUBNET_1 255
#define SUBNET_2 255
#define SUBNET_3 255
#define SUBNET_4 0
#define DNS1_1 1
#define DNS1_2 1
#define DNS1_3 1
#define DNS1_4 1
#define DNS2_1 8
#define DNS2_2 8
#define DNS2_3 8
#define DNS2_4 8

// Replace with the correct trusted PEM root certificate for api.telegram.org.
// Do not use WiFiClientSecure::setInsecure() in production.
#define TELEGRAM_CERTIFICATE_ROOT "REPLACE_WITH_VALID_TELEGRAM_ROOT_CA_PEM"
