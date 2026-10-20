/*
 * esp32RemoteCam - AI Thinker ESP32-CAM
 * Arduino-ESP32 core 2.x/3.x; install UniversalTelegramBot and ArduinoJson.
 * Copy config.example.h to config.h and fill in local settings/secrets.
 */
#include "config.h"
#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include "esp_http_server.h"

#if !defined(WIFI_SSID) || !defined(WIFI_PASSWORD) || !defined(TELEGRAM_BOT_TOKEN) || !defined(TELEGRAM_CHAT_ID)
#error "Create config.h from config.example.h and configure credentials."
#endif

static camera_config_t cameraConfig = {
  .pin_pwdn = 32, .pin_reset = -1, .pin_xclk = 0,
  .pin_sccb_sda = 26, .pin_sccb_scl = 27,
  .pin_d7 = 35, .pin_d6 = 34, .pin_d5 = 39, .pin_d4 = 36,
  .pin_d3 = 21, .pin_d2 = 19, .pin_d1 = 18, .pin_d0 = 5,
  .pin_vsync = 25, .pin_href = 23, .pin_pclk = 22,
  .xclk_freq_hz = 20000000, .ledc_timer = LEDC_TIMER_0,
  .ledc_channel = LEDC_CHANNEL_0, .pixel_format = PIXFORMAT_JPEG,
  .frame_size = FRAMESIZE_VGA, .jpeg_quality = 12, .fb_count = 2,
  .fb_location = CAMERA_FB_IN_PSRAM, .grab_mode = CAMERA_GRAB_LATEST
};

WiFiClientSecure telegramClient;
UniversalTelegramBot bot(TELEGRAM_BOT_TOKEN, telegramClient);
httpd_handle_t cameraHttpd = nullptr;
static camera_fb_t *telegramPhotoFrame = nullptr;
static size_t telegramPhotoIndex = 0;
unsigned long lastBotPoll = 0;
unsigned long lastWifiCheck = 0;
const uint32_t BOT_POLL_MS = 1200;
const uint32_t WIFI_CHECK_MS = 10000;

static esp_err_t rootHandler(httpd_req_t *req) {
  static const char PAGE[] PROGMEM =
    "<!doctype html><html><head><meta name='viewport' content='width=device-width,initial-scale=1'>"
    "<title>esp32RemoteCam</title><style>body{font:16px system-ui;max-width:800px;margin:2rem auto;padding:0 1rem;background:#111;color:#eee}"
    "img{width:100%;height:auto}a{color:#fff;background:#2457a7;padding:.7rem;border-radius:6px;margin:.3rem;display:inline-block}</style></head>"
    "<body><h1>esp32RemoteCam</h1><p>Camera is online on your local network.</p>"
    "<p><a href='/stream'>Open live stream</a> <a href='/capture'>Snapshot</a> <a href='/status'>Status JSON</a></p>"
    "<img src='/stream' alt='Live camera stream'></body></html>";
  httpd_resp_set_type(req, "text/html; charset=utf-8");
  httpd_resp_set_hdr(req, "Cache-Control", "no-store");
  return httpd_resp_send(req, PAGE, HTTPD_RESP_USE_STRLEN);
}

static esp_err_t statusHandler(httpd_req_t *req) {
  char json[256];
  snprintf(json, sizeof(json),
    "{\"ok\":true,\"ip\":\"%s\",\"rssi\":%d,\"uptime_ms\":%lu,\"heap_free\":%u}",
    WiFi.localIP().toString().c_str(), WiFi.RSSI(), millis(), ESP.getFreeHeap());
  httpd_resp_set_type(req, "application/json");
  httpd_resp_set_hdr(req, "Cache-Control", "no-store");
  return httpd_resp_send(req, json, HTTPD_RESP_USE_STRLEN);
}

static esp_err_t captureHandler(httpd_req_t *req) {
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Camera capture failed");
    return ESP_FAIL;
  }
  httpd_resp_set_type(req, "image/jpeg");
  httpd_resp_set_hdr(req, "Cache-Control", "no-store");
  esp_err_t result = httpd_resp_send(req, (const char *)fb->buf, fb->len);
  esp_camera_fb_return(fb);
  return result;
}

static esp_err_t streamHandler(httpd_req_t *req) {
  static const char *BOUNDARY = "\r\n--frame\r\n";
  static const char *TYPE = "multipart/x-mixed-replace;boundary=frame";
  httpd_resp_set_type(req, TYPE);
  httpd_resp_set_hdr(req, "Cache-Control", "no-store, no-cache, must-revalidate");
  while (true) {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) return ESP_FAIL;
    char header[96];
    int n = snprintf(header, sizeof(header),
      "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n", (unsigned)fb->len);
    esp_err_t res = httpd_resp_send_chunk(req, BOUNDARY, strlen(BOUNDARY));
    if (res == ESP_OK) res = httpd_resp_send_chunk(req, header, n);
    if (res == ESP_OK) res = httpd_resp_send_chunk(req, (const char *)fb->buf, fb->len);
    esp_camera_fb_return(fb);
    if (res != ESP_OK) break;
    vTaskDelay(pdMS_TO_TICKS(30));
  }
  httpd_resp_send_chunk(req, nullptr, 0);
  return ESP_OK;
}

static bool startCameraServer() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;
  config.ctrl_port = 32769;
  config.max_uri_handlers = 8;
  config.stack_size = 8192;
  if (httpd_start(&cameraHttpd, &config) != ESP_OK) return false;
  httpd_uri_t root = {.uri="/", .method=HTTP_GET, .handler=rootHandler, .user_ctx=nullptr};
  httpd_uri_t status = {.uri="/status", .method=HTTP_GET, .handler=statusHandler, .user_ctx=nullptr};
  httpd_uri_t capture = {.uri="/capture", .method=HTTP_GET, .handler=captureHandler, .user_ctx=nullptr};
  httpd_uri_t stream = {.uri="/stream", .method=HTTP_GET, .handler=streamHandler, .user_ctx=nullptr};
  httpd_register_uri_handler(cameraHttpd, &root);
  httpd_register_uri_handler(cameraHttpd, &status);
  httpd_register_uri_handler(cameraHttpd, &capture);
  httpd_register_uri_handler(cameraHttpd, &stream);
  return true;
}

static bool photoDataAvailable() {
  return telegramPhotoFrame != nullptr && telegramPhotoIndex < telegramPhotoFrame->len;
}
static byte photoNextByte() {
  return telegramPhotoFrame->buf[telegramPhotoIndex++];
}

static bool sendTelegramPhoto(const String &chatId) {
  telegramPhotoFrame = esp_camera_fb_get();
  if (!telegramPhotoFrame) {
    bot.sendMessage(chatId, "Camera capture failed.", "");
    return false;
  }
  telegramPhotoIndex = 0;
  String response = bot.sendPhotoByBinary(chatId, "image/jpeg", telegramPhotoFrame->len,
                                          photoDataAvailable, photoNextByte, nullptr, nullptr);
  bool ok = bot.checkForOkResponse(response);
  esp_camera_fb_return(telegramPhotoFrame);
  telegramPhotoFrame = nullptr;
  telegramPhotoIndex = 0;
  if (!ok) bot.sendMessage(chatId, "Telegram photo upload failed. Check TLS certificate and bot connectivity.", "");
  return ok;
}

static bool authorized(const String &chatId) {
  return chatId == String(TELEGRAM_CHAT_ID);
}

static void sendHelp(const String &chatId) {
  bot.sendMessage(chatId,
    "esp32RemoteCam commands:\n/status - network and camera status\n/ip - camera LAN URL\n/photo - local snapshot URL\n/flash_on - turn flash on\n/flash_off - turn flash off\n/help - this help",
    "");
}

static void handleTelegramMessages(int count) {
  for (int i = 0; i < count; i++) {
    String chatId = bot.messages[i].chat_id;
    String text = bot.messages[i].text;
    if (!authorized(chatId)) continue;
    if (text == "/start" || text == "/help") {
      sendHelp(chatId);
    } else if (text == "/ip") {
      bot.sendMessage(chatId, "Camera LAN URL: http://" + WiFi.localIP().toString() + "/\nStream: http://" + WiFi.localIP().toString() + "/stream", "");
    } else if (text == "/status") {
      String msg = "esp32RemoteCam: online\nIP: " + WiFi.localIP().toString() +
        "\nWi-Fi RSSI: " + String(WiFi.RSSI()) + " dBm\nUptime: " +
        String(millis() / 1000) + " s\nFree heap: " + String(ESP.getFreeHeap());
      bot.sendMessage(chatId, msg, "");
    } else if (text == "/photo") {
      bot.sendMessage(chatId, "Open this snapshot URL on your local network: http://" + WiFi.localIP().toString() + "/capture", "");
    } else if (text == "/flash_on" || text == "/flash_off") {
      pinMode(4, OUTPUT);
      digitalWrite(4, text == "/flash_on" ? HIGH : LOW);
      bot.sendMessage(chatId, text == "/flash_on" ? "Flash on." : "Flash off.", "");
    } else {
      bot.sendMessage(chatId, "Unknown command. Send /help.", "");
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(300);
  if (psramFound()) {
    cameraConfig.fb_count = 2;
    cameraConfig.frame_size = FRAMESIZE_VGA;
  } else {
    cameraConfig.fb_count = 1;
    cameraConfig.frame_size = FRAMESIZE_QVGA;
  }
  esp_err_t cameraResult = esp_camera_init(&cameraConfig);
  if (cameraResult != ESP_OK) {
    Serial.printf("Camera init failed: 0x%x\n", cameraResult);
    while (true) delay(1000);
  }
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
#ifdef USE_STATIC_IP
  IPAddress localIp(STATIC_IP_1, STATIC_IP_2, STATIC_IP_3, STATIC_IP_4);
  IPAddress gateway(GATEWAY_1, GATEWAY_2, GATEWAY_3, GATEWAY_4);
  IPAddress subnet(SUBNET_1, SUBNET_2, SUBNET_3, SUBNET_4);
  IPAddress dns1(DNS1_1, DNS1_2, DNS1_3, DNS1_4);
  IPAddress dns2(DNS2_1, DNS2_2, DNS2_3, DNS2_4);
  if (!WiFi.config(localIp, gateway, subnet, dns1, dns2)) Serial.println("Static IP config failed; using DHCP.");
#endif
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  uint32_t started = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - started < 30000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi connection failed; restarting.");
    delay(1000);
    ESP.restart();
  }
  Serial.print("Camera IP: http://");
  Serial.println(WiFi.localIP());
  // Configure a valid trusted root CA for api.telegram.org; never use setInsecure().
  telegramClient.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  bot.longPoll = 2;
  if (!startCameraServer()) Serial.println("HTTP server failed to start");
  bot.sendMessage(String(TELEGRAM_CHAT_ID), "esp32RemoteCam online at http://" + WiFi.localIP().toString(), "");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED && millis() - lastWifiCheck >= WIFI_CHECK_MS) {
    lastWifiCheck = millis();
    WiFi.reconnect();
  }
  if (millis() - lastBotPoll >= BOT_POLL_MS && WiFi.status() == WL_CONNECTED) {
    lastBotPoll = millis();
    int count = bot.getUpdates(bot.last_message_received + 1);
    while (count) {
      handleTelegramMessages(count);
      count = bot.getUpdates(bot.last_message_received + 1);
    }
  }
  delay(5);
}
