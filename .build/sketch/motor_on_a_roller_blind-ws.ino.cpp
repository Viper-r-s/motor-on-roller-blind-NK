#include <Arduino.h>
#line 1 "G:\\Meine Ablage\\Sicherungen\\Raspi-Sicherungen\\Node MCU ESP8266\\motor-on-roller-blind-ws-master\\v2\\motor_on_a_roller_blind-ws\\motor_on_a_roller_blind-ws.ino"
#include <Stepper_28BYJ_48.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <PubSubClient.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <flash_hal.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoOTA.h>
#include <Updater.h>
#include "NidayandHelper.h"
#include "BlindMotion.h"
#include "StatusSchedule.h"
#include "FirmwareUpload.h"
#include "DeviceName.h"
#include "index_html.h"

// Keep the original AP credentials and motor pin order.
const char* APid = "F1";
const char* APpw = "12345678";
const char* version = "1.7.0";

struct DeviceConfig {
  long currentPosition = 0;
  long startPosition = 0;
  long maxPosition = 2000000;
  char name[40] = "roller-blind";
  char mqttServer[40] = "";
  char mqttPort[6] = "1883";
  char mqttUser[40] = "";
  char mqttPassword[40] = "";
  bool clockwise = false;
};

DeviceConfig config; // Last successfully saved values, not live motor position.
BlindMotion motion;
NidayandHelper helper;
WiFiClient espClient;
PubSubClient psclient(espClient);
Stepper_28BYJ_48 small_stepper(D1, D3, D2, D4);
ESP8266WebServer server(80);
WebSocketsServer webSocket(81);
String outputTopic, inputTopic;
bool filesystemReady = false;
bool persisted = false;
bool shouldSaveConfig = false;
unsigned long configRevision = 0;
StatusSchedule statusSchedule;
String saveError;
FirmwareUpload firmwareUpload;
String firmwareToken;
unsigned long firmwareRestartRequestedAt = 0;
bool nameRestartPending = false;
unsigned long nameRestartRequestedAt = 0;
struct FlashFirmwareWriter {
  bool begin(size_t size) { return Update.begin(size, U_FLASH); }
  size_t write(uint8_t* data, size_t length) { return Update.write(data, length); }
  bool end(bool partial) { return Update.end(partial); }
} firmwareWriter;

#line 63 "G:\\Meine Ablage\\Sicherungen\\Raspi-Sicherungen\\Node MCU ESP8266\\motor-on-roller-blind-ws-master\\v2\\motor_on_a_roller_blind-ws\\motor_on_a_roller_blind-ws.ino"
size_t firmwareCapacity();
#line 68 "G:\\Meine Ablage\\Sicherungen\\Raspi-Sicherungen\\Node MCU ESP8266\\motor-on-roller-blind-ws-master\\v2\\motor_on_a_roller_blind-ws\\motor_on_a_roller_blind-ws.ino"
bool updateBusy();
#line 70 "G:\\Meine Ablage\\Sicherungen\\Raspi-Sicherungen\\Node MCU ESP8266\\motor-on-roller-blind-ws-master\\v2\\motor_on_a_roller_blind-ws\\motor_on_a_roller_blind-ws.ino"
void copyJsonString(JsonVariantConst json, const char* key, char* destination, size_t size);
#line 75 "G:\\Meine Ablage\\Sicherungen\\Raspi-Sicherungen\\Node MCU ESP8266\\motor-on-roller-blind-ws-master\\v2\\motor_on_a_roller_blind-ws\\motor_on_a_roller_blind-ws.ino"
bool loadConfig();
#line 100 "G:\\Meine Ablage\\Sicherungen\\Raspi-Sicherungen\\Node MCU ESP8266\\motor-on-roller-blind-ws-master\\v2\\motor_on_a_roller_blind-ws\\motor_on_a_roller_blind-ws.ino"
bool saveConfig(const DeviceConfig& candidate);
#line 122 "G:\\Meine Ablage\\Sicherungen\\Raspi-Sicherungen\\Node MCU ESP8266\\motor-on-roller-blind-ws-master\\v2\\motor_on_a_roller_blind-ws\\motor_on_a_roller_blind-ws.ino"
void stopPowerToCoils();
#line 127 "G:\\Meine Ablage\\Sicherungen\\Raspi-Sicherungen\\Node MCU ESP8266\\motor-on-roller-blind-ws-master\\v2\\motor_on_a_roller_blind-ws\\motor_on_a_roller_blind-ws.ino"
void haltMotor();
#line 132 "G:\\Meine Ablage\\Sicherungen\\Raspi-Sicherungen\\Node MCU ESP8266\\motor-on-roller-blind-ws-master\\v2\\motor_on_a_roller_blind-ws\\motor_on_a_roller_blind-ws.ino"
bool savePosition();
#line 138 "G:\\Meine Ablage\\Sicherungen\\Raspi-Sicherungen\\Node MCU ESP8266\\motor-on-roller-blind-ws-master\\v2\\motor_on_a_roller_blind-ws\\motor_on_a_roller_blind-ws.ino"
String statusJson();
#line 159 "G:\\Meine Ablage\\Sicherungen\\Raspi-Sicherungen\\Node MCU ESP8266\\motor-on-roller-blind-ws-master\\v2\\motor_on_a_roller_blind-ws\\motor_on_a_roller_blind-ws.ino"
String configJson();
#line 195 "G:\\Meine Ablage\\Sicherungen\\Raspi-Sicherungen\\Node MCU ESP8266\\motor-on-roller-blind-ws-master\\v2\\motor_on_a_roller_blind-ws\\motor_on_a_roller_blind-ws.ino"
void configureMqtt();
#line 202 "G:\\Meine Ablage\\Sicherungen\\Raspi-Sicherungen\\Node MCU ESP8266\\motor-on-roller-blind-ws-master\\v2\\motor_on_a_roller_blind-ws\\motor_on_a_roller_blind-ws.ino"
bool processMsg(String message, String& error);
#line 242 "G:\\Meine Ablage\\Sicherungen\\Raspi-Sicherungen\\Node MCU ESP8266\\motor-on-roller-blind-ws-master\\v2\\motor_on_a_roller_blind-ws\\motor_on_a_roller_blind-ws.ino"
void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length);
#line 259 "G:\\Meine Ablage\\Sicherungen\\Raspi-Sicherungen\\Node MCU ESP8266\\motor-on-roller-blind-ws-master\\v2\\motor_on_a_roller_blind-ws\\motor_on_a_roller_blind-ws.ino"
void mqttCallback(char* topic, byte* payload, unsigned int length);
#line 267 "G:\\Meine Ablage\\Sicherungen\\Raspi-Sicherungen\\Node MCU ESP8266\\motor-on-roller-blind-ws-master\\v2\\motor_on_a_roller_blind-ws\\motor_on_a_roller_blind-ws.ino"
void respondError(int code, const String& error);
#line 276 "G:\\Meine Ablage\\Sicherungen\\Raspi-Sicherungen\\Node MCU ESP8266\\motor-on-roller-blind-ws-master\\v2\\motor_on_a_roller_blind-ws\\motor_on_a_roller_blind-ws.ino"
void handleFirmwareUpload();
#line 316 "G:\\Meine Ablage\\Sicherungen\\Raspi-Sicherungen\\Node MCU ESP8266\\motor-on-roller-blind-ws-master\\v2\\motor_on_a_roller_blind-ws\\motor_on_a_roller_blind-ws.ino"
void handleFirmwareComplete();
#line 328 "G:\\Meine Ablage\\Sicherungen\\Raspi-Sicherungen\\Node MCU ESP8266\\motor-on-roller-blind-ws-master\\v2\\motor_on_a_roller_blind-ws\\motor_on_a_roller_blind-ws.ino"
bool rejectDuringUpdate();
#line 334 "G:\\Meine Ablage\\Sicherungen\\Raspi-Sicherungen\\Node MCU ESP8266\\motor-on-roller-blind-ws-master\\v2\\motor_on_a_roller_blind-ws\\motor_on_a_roller_blind-ws.ino"
void handleDeviceName();
#line 360 "G:\\Meine Ablage\\Sicherungen\\Raspi-Sicherungen\\Node MCU ESP8266\\motor-on-roller-blind-ws-master\\v2\\motor_on_a_roller_blind-ws\\motor_on_a_roller_blind-ws.ino"
void handleLimits();
#line 380 "G:\\Meine Ablage\\Sicherungen\\Raspi-Sicherungen\\Node MCU ESP8266\\motor-on-roller-blind-ws-master\\v2\\motor_on_a_roller_blind-ws\\motor_on_a_roller_blind-ws.ino"
void handleDirection();
#line 393 "G:\\Meine Ablage\\Sicherungen\\Raspi-Sicherungen\\Node MCU ESP8266\\motor-on-roller-blind-ws-master\\v2\\motor_on_a_roller_blind-ws\\motor_on_a_roller_blind-ws.ino"
void handleMqttSettings();
#line 427 "G:\\Meine Ablage\\Sicherungen\\Raspi-Sicherungen\\Node MCU ESP8266\\motor-on-roller-blind-ws-master\\v2\\motor_on_a_roller_blind-ws\\motor_on_a_roller_blind-ws.ino"
void saveConfigCallback();
#line 429 "G:\\Meine Ablage\\Sicherungen\\Raspi-Sicherungen\\Node MCU ESP8266\\motor-on-roller-blind-ws-master\\v2\\motor_on_a_roller_blind-ws\\motor_on_a_roller_blind-ws.ino"
void setup();
#line 518 "G:\\Meine Ablage\\Sicherungen\\Raspi-Sicherungen\\Node MCU ESP8266\\motor-on-roller-blind-ws-master\\v2\\motor_on_a_roller_blind-ws\\motor_on_a_roller_blind-ws.ino"
void loop();
#line 63 "G:\\Meine Ablage\\Sicherungen\\Raspi-Sicherungen\\Node MCU ESP8266\\motor-on-roller-blind-ws-master\\v2\\motor_on_a_roller_blind-ws\\motor_on_a_roller_blind-ws.ino"
size_t firmwareCapacity() {
  uint32_t freeSpace = ESP.getFreeSketchSpace();
  return freeSpace > 0x1000 ? (freeSpace - 0x1000) & 0xFFFFF000UL : 0;
}

bool updateBusy() { return nameRestartPending || firmwareUpload.busy() || Update.isRunning(); }

void copyJsonString(JsonVariantConst json, const char* key, char* destination, size_t size) {
  const char* value = json[key];
  if (value && strlen(value) < size) strlcpy(destination, value, size);
}

bool loadConfig() {
  if (!helper.loadconfig()) return false;
  JsonVariantConst json = helper.getconfig();
  if (json["currentPosition"].is<long>()) config.currentPosition = json["currentPosition"];
  // Older configuration files have an implicit start position of zero.
  if (json["startPosition"].is<long>()) config.startPosition = json["startPosition"];
  if (json["maxPosition"].is<long>()) config.maxPosition = json["maxPosition"];
  if (!BlindMotion::validLimits(config.startPosition, config.maxPosition)) {
    config.startPosition = 0;
    config.maxPosition = 2000000;
  }
  if (config.currentPosition < -BlindMotion::coordinateLimit || config.currentPosition > BlindMotion::coordinateLimit) config.currentPosition = config.startPosition;
  copyJsonString(json, "config_name", config.name, sizeof(config.name));
  if (!config.name[0]) strlcpy(config.name, "roller-blind", sizeof(config.name));
  copyJsonString(json, "mqtt_server", config.mqttServer, sizeof(config.mqttServer));
  copyJsonString(json, "mqtt_port", config.mqttPort, sizeof(config.mqttPort));
  copyJsonString(json, "mqtt_uid", config.mqttUser, sizeof(config.mqttUser));
  copyJsonString(json, "mqtt_pwd", config.mqttPassword, sizeof(config.mqttPassword));
  const char* rotation = json["config_rotation"];
  config.clockwise = rotation && strcmp(rotation, "true") == 0;
  long port;
  if (!BlindMotion::parseNumber(config.mqttPort, 1, 65535, port)) strlcpy(config.mqttPort, "1883", sizeof(config.mqttPort));
  return true;
}

bool saveConfig(const DeviceConfig& candidate) {
  JsonDocument json;
  json["currentPosition"] = candidate.currentPosition;
  json["startPosition"] = candidate.startPosition;
  json["maxPosition"] = candidate.maxPosition;
  json["config_name"] = candidate.name;
  json["mqtt_server"] = candidate.mqttServer;
  json["mqtt_port"] = candidate.mqttPort;
  json["mqtt_uid"] = candidate.mqttUser;
  json["mqtt_pwd"] = candidate.mqttPassword;
  json["config_rotation"] = candidate.clockwise ? "true" : "false";
  if (json.overflowed() || !filesystemReady || !helper.saveconfig(json.as<JsonVariantConst>())) {
    saveError = "Failed to save to flash. Previous settings have been retained.";
    return false;
  }
  config = candidate;
  persisted = true;
  ++configRevision;
  saveError = "";
  return true;
}

void stopPowerToCoils() {
  digitalWrite(D1, LOW); digitalWrite(D2, LOW);
  digitalWrite(D3, LOW); digitalWrite(D4, LOW);
}

void haltMotor() {
  motion.stop();
  stopPowerToCoils();
}

bool savePosition() {
  DeviceConfig candidate = config;
  candidate.currentPosition = motion.current;
  return saveConfig(candidate);
}

String statusJson() {
  JsonDocument json;
  json["type"] = "status";
  json["set"] = motion.percent(motion.target);
  json["position"] = motion.percent(motion.current);
  json["currentPosition"] = motion.current;
  json["targetPosition"] = motion.target;
  json["startPosition"] = motion.start;
  json["maxPosition"] = motion.end;
  json["moving"] = motion.moving();
  int direction = motion.direction();
  json["direction"] = direction > 0 ? "down" : direction < 0 ? "up" : "stopped";
  json["clockwise"] = config.clockwise;
  json["mqttConnected"] = psclient.connected();
  json["configRevision"] = configRevision;
  json["saveError"] = saveError;
  String result;
  serializeJson(json, result);
  return result;
}

String configJson() {
  JsonDocument json;
  json["persisted"] = persisted;
  json["configRevision"] = configRevision;
  json["name"] = config.name;
  json["ipAddress"] = WiFi.localIP().toString();
  json["restartPending"] = nameRestartPending;
  json["version"] = version;
  json["firmwareMaxSize"] = firmwareCapacity();
  json["firmwareToken"] = firmwareToken;
  json["firmwareUpdating"] = firmwareUpload.busy() || Update.isRunning();
  json["currentPosition"] = config.currentPosition;
  json["startPosition"] = config.startPosition;
  json["maxPosition"] = config.maxPosition;
  json["clockwise"] = config.clockwise;
  json["mqtt_server"] = config.mqttServer;
  json["mqtt_port"] = config.mqttPort;
  json["mqtt_uid"] = config.mqttUser;
  // Never disclose the stored password to browsers, WebSockets or MQTT.
  json["mqtt_password_set"] = config.mqttPassword[0] != '\0';
  json["inputTopic"] = inputTopic;
  json["outputTopic"] = outputTopic;
  String result;
  serializeJson(json, result);
  return result;
}

void publishStatus(bool mqtt = true) {
  String payload = statusJson();
  webSocket.broadcastTXT(payload);
  statusSchedule.sent(millis(), mqtt);
  if (mqtt && psclient.connected()) {
    helper.mqtt_publish(psclient, outputTopic, payload);
  }
}

void configureMqtt() {
  psclient.disconnect();
  espClient.stop();
  psclient.setServer(config.mqttServer, atoi(config.mqttPort));
}

// Both MQTT and WebSocket keep supporting the original plain-text commands.
bool processMsg(String message, String& error) {
  if (message == "(ping)") return true;
  if (message == "(update)") { publishStatus(); return true; }
  if (updateBusy()) { error = "An update or restart is in progress. Motor control is locked."; return false; }
  if (message == "(0)") {
    haltMotor();
    bool ok = savePosition();
    if (!ok) error = saveError;
    publishStatus();
    return ok;
  }
  if (message == "(start)" || message == "(max)") {
    haltMotor();
    DeviceConfig candidate = config;
    candidate.currentPosition = motion.current;
    if (message == "(start)") candidate.startPosition = motion.current;
    else candidate.maxPosition = motion.current;
    if (!BlindMotion::validLimits(candidate.startPosition, candidate.maxPosition)) {
      error = "Start must be less than end. You can also enter both limits directly.";
      publishStatus();
      return false;
    }
    if (!saveConfig(candidate)) { error = saveError; publishStatus(); return false; }
    motion.start = config.startPosition;
    motion.end = config.maxPosition;
  } else if (message == "(1)" || message == "(-1)") {
    motion.manual(message == "(1)" ? 1 : -1);
  } else {
    long percentage;
    if (!BlindMotion::parseNumber(message.c_str(), 0, 100, percentage)) {
      error = "Unknown command. Position must be an integer from 0 to 100.";
      return false;
    }
    motion.goTo(percentage);
    if (!motion.moving()) { haltMotor(); if (!savePosition()) error = saveError; }
  }
  publishStatus();
  return error.length() == 0;
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
  if (type == WStype_CONNECTED) {
    String status = statusJson();
    webSocket.sendTXT(num, status);
  } else if (type == WStype_TEXT && length <= 64) {
    String message;
    for (size_t i = 0; i < length; ++i) message += char(payload[i]);
    String error;
    if (!processMsg(message, error)) {
      JsonDocument json;
      json["error"] = error;
      String result; serializeJson(json, result);
      webSocket.sendTXT(num, result);
    }
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  (void)topic; // One configured command topic; PubSubClient requires this argument.
  if (length > 64) return;
  String message, error;
  for (unsigned int i = 0; i < length; ++i) message += char(payload[i]);
  if (!processMsg(message, error)) Serial.println("MQTT-Befehl abgelehnt: " + error);
}

void respondError(int code, const String& error) {
  JsonDocument json;
  json["error"] = error;
  String result; serializeJson(json, result);
  server.send(code, "application/json; charset=utf-8", result);
}

// Custom streaming endpoint: never accepts filesystem images and only commits
// after the complete multipart request (not merely the first FILE_END).
void handleFirmwareUpload() {
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    if (nameRestartPending) {
      firmwareUpload.reject(firmwareWriter, 409, "A device-name change is pending. Wait for the restart."); return;
    }
    if (firmwareUpload.busy()) {
      firmwareUpload.reject(firmwareWriter, 409, "Only one firmware file is allowed per request.");
      return;
    }
    if (Update.isRunning()) {
      firmwareUpload.reject(firmwareWriter, 409, "Another update is already in progress."); return;
    }
    if (!firmwareToken.length() || server.header("X-Firmware-Token") != firmwareToken) {
      firmwareUpload.reject(firmwareWriter, 403, "Update token missing or expired. Reload this page."); return;
    }
    if (upload.name != "firmware" || !FirmwareUpload::validName(upload.filename.c_str())) {
      firmwareUpload.reject(firmwareWriter, 400, "Select exactly one firmware file (.bin)."); return;
    }
    long size;
    if (!BlindMotion::parseNumber(server.header("X-Firmware-Size").c_str(), 32, firmwareCapacity(), size)) {
      firmwareUpload.reject(firmwareWriter, 413, "Invalid file size or insufficient update space."); return;
    }
    haltMotor();
    if (!savePosition()) {
      firmwareUpload.reject(firmwareWriter, 500, "Could not save the position. Firmware update cancelled."); return;
    }
    publishStatus();
    firmwareUpload.begin(firmwareWriter, size, firmwareCapacity());
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    firmwareUpload.write(firmwareWriter, upload.buf, upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    firmwareUpload.fileEnd(firmwareWriter, upload.totalSize);
  } else if (upload.status == UPLOAD_FILE_ABORTED) {
    firmwareUpload.reject(firmwareWriter, 400, "Upload cancelled. The current firmware remains active.");
    haltMotor();
  }
  yield();
}

void handleFirmwareComplete() {
  if (!firmwareUpload.finish(firmwareWriter)) {
    respondError(firmwareUpload.errorCode, firmwareUpload.error);
    return;
  }
  server.sendHeader("Cache-Control", "no-store");
  server.sendHeader("Connection", "close");
  server.client().setNoDelay(true);
  server.send(200, "application/json; charset=utf-8", "{\"ok\":true,\"rebooting\":true}");
  firmwareRestartRequestedAt = millis();
}

bool rejectDuringUpdate() {
  if (!updateBusy()) return false;
  respondError(409, "An update or restart is in progress. Settings and motor control are locked.");
  return true;
}

void handleDeviceName() {
  if (rejectDuringUpdate()) return;
  String name = server.arg("name");
  name.trim();
  name.toLowerCase();
  if (!DeviceName::valid(name.c_str())) {
    respondError(400, "Use 1-31 letters, digits or hyphens. No spaces, dots, leading or trailing hyphens.");
    return;
  }
  if (name == config.name) {
    server.send(200, "application/json; charset=utf-8", configJson());
    return;
  }
  haltMotor();
  DeviceConfig candidate = config;
  candidate.currentPosition = motion.current;
  name.toCharArray(candidate.name, sizeof(candidate.name));
  if (!saveConfig(candidate)) { respondError(500, saveError); publishStatus(); return; }
  // ArduinoOTA's setHostname() ignores changes after initialization. A delayed
  // restart applies the saved name consistently to DHCP, mDNS and ArduinoOTA.
  nameRestartPending = true;
  nameRestartRequestedAt = millis();
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json; charset=utf-8", configJson());
}

void handleLimits() {
  if (rejectDuringUpdate()) return;
  long first, last;
  if (!BlindMotion::parseNumber(server.arg("startPosition").c_str(), -BlindMotion::coordinateLimit, BlindMotion::coordinateLimit, first) ||
      !BlindMotion::parseNumber(server.arg("maxPosition").c_str(), -BlindMotion::coordinateLimit, BlindMotion::coordinateLimit, last) ||
      !BlindMotion::validLimits(first, last)) {
    respondError(400, "Invalid limits: start < end; range -1000000000 to 1000000000 steps.");
    return;
  }
  haltMotor();
  DeviceConfig candidate = config;
  candidate.currentPosition = motion.current;
  candidate.startPosition = first;
  candidate.maxPosition = last;
  if (!saveConfig(candidate)) { respondError(500, saveError); publishStatus(); return; }
  motion.start = first; motion.end = last;
  server.send(200, "application/json; charset=utf-8", configJson());
  publishStatus();
}

void handleDirection() {
  if (rejectDuringUpdate()) return;
  String value = server.arg("clockwise");
  if (value != "true" && value != "false") { respondError(400, "Invalid rotation direction."); return; }
  haltMotor();
  DeviceConfig candidate = config;
  candidate.currentPosition = motion.current;
  candidate.clockwise = value == "true";
  if (!saveConfig(candidate)) { respondError(500, saveError); publishStatus(); return; }
  server.send(200, "application/json; charset=utf-8", configJson());
  publishStatus();
}

void handleMqttSettings() {
  if (rejectDuringUpdate()) return;
  long port;
  String host = server.arg("mqtt_server"), uid = server.arg("mqtt_uid"), password = server.arg("mqtt_pwd");
  host.trim();
  if (!server.hasArg("mqtt_server") || !server.hasArg("mqtt_uid") ||
      !BlindMotion::parseNumber(server.arg("mqtt_port").c_str(), 1, 65535, port) ||
      host.length() >= sizeof(config.mqttServer) || uid.length() >= sizeof(config.mqttUser) || password.length() >= sizeof(config.mqttPassword)) {
    respondError(400, "MQTT: host, username and password must not exceed 39 characters; port must be 1-65535.");
    return;
  }
  for (size_t i = 0; i < host.length(); ++i) {
    char c = host[i];
    if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '.' || c == '-')) {
      respondError(400, "Enter the broker IPv4 address or hostname without a protocol or path."); return;
    }
  }
  if (password.length() && server.arg("clear_password") == "true") {
    respondError(400, "Either enter a new password or select delete password."); return;
  }
  haltMotor();
  DeviceConfig candidate = config;
  candidate.currentPosition = motion.current;
  host.toCharArray(candidate.mqttServer, sizeof(candidate.mqttServer));
  String(port).toCharArray(candidate.mqttPort, sizeof(candidate.mqttPort));
  uid.toCharArray(candidate.mqttUser, sizeof(candidate.mqttUser));
  if (password.length()) password.toCharArray(candidate.mqttPassword, sizeof(candidate.mqttPassword));
  else if (server.arg("clear_password") == "true") candidate.mqttPassword[0] = '\0';
  if (!saveConfig(candidate)) { respondError(500, saveError); publishStatus(); return; }
  configureMqtt();
  server.send(200, "application/json; charset=utf-8", configJson());
  publishStatus();
}

void saveConfigCallback() { shouldSaveConfig = true; }

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println(F("[boot] Roller blind controller startet"));
  Serial.println(ESP.getResetInfo());
  haltMotor();
  Serial.println(F("[boot] SPIFFS einbinden"));
  filesystemReady = helper.beginStorage(FS_PHYS_ADDR, FS_PHYS_ADDR + FS_PHYS_SIZE);
  Serial.println(filesystemReady ? F("[boot] SPIFFS bereit; Konfiguration laden") : F("[boot] SPIFFS nicht verfuegbar"));
  if (filesystemReady) persisted = loadConfig();
  else saveError = "Filesystem not mounted: check the flash layout (4 MB / 1 MB FS). Existing data was not formatted.";
  Serial.println(F("[boot] Konfiguration abgeschlossen"));
  motion.current = config.currentPosition;
  motion.start = config.startPosition;
  motion.end = config.maxPosition;
  motion.stop();
  inputTopic = helper.mqtt_gettopic("in");
  outputTopic = helper.mqtt_gettopic("out");

  // Load saved values BEFORE creating WiFiManager parameters, not afterwards.
  WiFi.hostname(config.name);
  char rotation[6]; strlcpy(rotation, config.clockwise ? "true" : "false", sizeof(rotation));
  WiFiManagerParameter nameParam("Name", "Device name (hostname)", config.name, 31);
  WiFiManagerParameter rotationParam("Rotation", "Clockwise to close: true/false", rotation, 5);
  WiFiManagerParameter serverParam("server", "MQTT server", config.mqttServer, 39);
  WiFiManagerParameter portParam("port", "MQTT port", config.mqttPort, 5);
  WiFiManagerParameter userParam("uid", "MQTT username", config.mqttUser, 39);
  // Keep an existing password unless a replacement was entered in the portal.
  WiFiManagerParameter passwordParam("pwd", "MQTT password (blank: keep)", "", 39, "type=\"password\"");
  WiFiManager wifiManager;
  wifiManager.setHostname(config.name);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.addParameter(&nameParam); wifiManager.addParameter(&rotationParam);
  wifiManager.addParameter(&serverParam); wifiManager.addParameter(&portParam);
  wifiManager.addParameter(&userParam); wifiManager.addParameter(&passwordParam);
  Serial.println(F("[boot] WLAN verbinden / Konfigurationsportal starten"));
  wifiManager.autoConnect(APid, APpw);
  Serial.println(F("[boot] WLAN-Konfiguration abgeschlossen"));
  if (shouldSaveConfig) {
    DeviceConfig candidate = config;
    strlcpy(candidate.name, nameParam.getValue(), sizeof(candidate.name));
    // Preserve the previous name if the captive portal submitted an invalid one.
    String portalName = candidate.name;
    portalName.trim(); portalName.toLowerCase();
    if (DeviceName::valid(portalName.c_str())) portalName.toCharArray(candidate.name, sizeof(candidate.name));
    else strlcpy(candidate.name, config.name, sizeof(candidate.name));
    strlcpy(candidate.mqttServer, serverParam.getValue(), sizeof(candidate.mqttServer));
    strlcpy(candidate.mqttPort, portParam.getValue(), sizeof(candidate.mqttPort));
    strlcpy(candidate.mqttUser, userParam.getValue(), sizeof(candidate.mqttUser));
    if (passwordParam.getValue()[0]) strlcpy(candidate.mqttPassword, passwordParam.getValue(), sizeof(candidate.mqttPassword));
    candidate.clockwise = strcmp(rotationParam.getValue(), "true") == 0;
    long port;
    if (!BlindMotion::parseNumber(candidate.mqttPort, 1, 65535, port)) strlcpy(candidate.mqttPort, "1883", sizeof(candidate.mqttPort));
    saveConfig(candidate);
  }
  WiFi.hostname(config.name);
  if (MDNS.begin(config.name)) {
    MDNS.addService("http", "tcp", 80);
    MDNS.addService("ws", "tcp", 81);
  }
  firmwareToken = String(ESP.random(), HEX) + String(ESP.random(), HEX) + String(ESP.random(), HEX);
  server.collectHeaders("X-Firmware-Token", "X-Firmware-Size");
  server.on("/api/firmware", HTTP_POST, handleFirmwareComplete, handleFirmwareUpload);
  server.on("/", HTTP_GET, []() { server.sendHeader("Cache-Control", "no-store"); server.send_P(200, "text/html; charset=utf-8", INDEX_HTML); });
  server.on("/api/status", HTTP_GET, []() { server.sendHeader("Cache-Control", "no-store"); server.send(200, "application/json; charset=utf-8", statusJson()); });
  server.on("/api/config", HTTP_GET, []() { server.sendHeader("Cache-Control", "no-store"); server.send(200, "application/json; charset=utf-8", configJson()); });
  server.on("/api/command", HTTP_POST, []() {
    String error;
    bool ok = processMsg(server.arg("command"), error);
    if (ok) server.send(200, "application/json; charset=utf-8", statusJson());
    else respondError(error == saveError && error.length() ? 500 : 400, error);
  });
  server.on("/api/name", HTTP_POST, handleDeviceName);
  server.on("/api/limits", HTTP_POST, handleLimits);
  server.on("/api/direction", HTTP_POST, handleDirection);
  server.on("/api/mqtt", HTTP_POST, handleMqttSettings);
  server.onNotFound([]() { respondError(404, "Not found."); });
  server.begin();
  webSocket.begin(); webSocket.onEvent(webSocketEvent);
  psclient.setCallback(mqttCallback);
  psclient.setBufferSize(768);
  psclient.setSocketTimeout(1);
  configureMqtt();
  ArduinoOTA.setHostname(config.name);
  ArduinoOTA.onStart([]() { haltMotor(); savePosition(); });
  ArduinoOTA.begin();
  Serial.println("Weboberflaeche: http://" + WiFi.localIP().toString());
}

void loop() {
  if (nameRestartPending) {
    haltMotor();
    if (uint32_t(millis() - nameRestartRequestedAt) >= 1200U) ESP.restart();
    yield(); return;
  }
  if (firmwareUpload.state == FirmwareUpload::Complete) {
    haltMotor();
    if (uint32_t(millis() - firmwareRestartRequestedAt) >= 1200U) ESP.restart();
    yield();
    return;
  }
  if (!firmwareUpload.busy()) ArduinoOTA.handle();
  MDNS.update();
  webSocket.loop();
  server.handleClient();
  if (nameRestartPending || firmwareUpload.busy()) { haltMotor(); yield(); return; }
  // Connection attempts can block inside the TCP library: do them only at rest.
  // An already connected broker continues receiving commands during motion.
  if (config.mqttServer[0]) {
    bool wasConnected = psclient.connected();
    if (wasConnected || !motion.moving()) helper.mqtt_reconnect(psclient, config.mqttUser, config.mqttPassword, { inputTopic.c_str() });
    if (!wasConnected && psclient.connected()) publishStatus();
  }

  if (motion.moving()) {
    int direction = motion.direction();
    small_stepper.step(config.clockwise ? -direction : direction);
    motion.current += direction;
    if (!motion.moving()) {
      haltMotor();
      savePosition();
      publishStatus();
    }
  }
  unsigned long now = millis();
  if (statusSchedule.mqttDue(now, motion.moving())) {
    publishStatus();
  } else if (statusSchedule.webDue(now, motion.moving())) {
    publishStatus(false);
  }
  yield();
}

