#line 1 "G:\\Meine Ablage\\Sicherungen\\Raspi-Sicherungen\\Node MCU ESP8266\\motor-on-roller-blind-ws-master\\v2\\motor_on_a_roller_blind-ws\\tests\\host\\mocks.h"
#pragma once
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <cstring>
#include <stdint.h>
using boolean = bool;
class String : public std::string {
 public:
  using std::string::string;
  String(const std::string& value) : std::string(value) {}
  String(unsigned int value) : std::string(std::to_string(value)) {}
};
struct ESPMock {
  std::vector<uint8_t> flash = std::vector<uint8_t>(4096, 255);
  bool readFails = false;
  unsigned int getChipId() { return 123; }
  bool flashRead(uint32_t address, uint32_t* target, size_t length) {
    if (readFails || address + length > flash.size()) return false;
    memcpy(target, flash.data() + address, length);
    return true;
  }
};
extern ESPMock ESP;
extern uint32_t fakeMillis;
inline unsigned long millis() { return fakeMillis; }
inline void yield() {}
const int WL_CONNECTED = 3;
struct WiFiMock {
  int connection = WL_CONNECTED;
  int status() { return connection; }
  struct IP { String toString() { return "192.168.1.5"; } };
  IP localIP() { return {}; }
};
extern WiFiMock WiFi;
struct File {
  std::string* data = nullptr;
  size_t cursor = 0;
  size_t maxWrite = SIZE_MAX;
  explicit operator bool() const { return data; }
  size_t size() { return data ? data->size() : 0; }
  size_t readBytes(char* target, size_t length) {
    size_t count = std::min(length, data->size() - cursor);
    memcpy(target, data->data() + cursor, count); cursor += count; return count;
  }
  size_t print(char value) {
    if (!data || data->size() >= maxWrite) return 0;
    *data += value; return 1;
  }
  size_t print(const char* value) {
    size_t result = 0;
    for (; *value; ++value) result += print(*value);
    return result;
  }
  size_t write(uint8_t value) { return print(char(value)); }
  size_t write(const uint8_t* values, size_t length) {
    size_t result = 0;
    for (size_t i = 0; i < length; ++i) result += write(values[i]);
    return result;
  }
  void flush() {}
  void close() {}
};
struct SPIFFSConfig {
  bool autoFormat = true;
  void setAutoFormat(bool value) { autoFormat = value; }
};
struct FSMock {
  std::map<std::string, std::string> files;
  bool mounted = true, formatSucceeds = true, autoFormat = true;
  unsigned formats = 0;
  void setConfig(SPIFFSConfig options) { autoFormat = options.autoFormat; }
  bool begin() { return mounted; }
  bool failOpenWrite = false;
  bool failInstall = false;
  size_t maxWrite = SIZE_MAX;
  File open(const char* path, const char* mode) {
    if (*mode == 'w') {
      if (failOpenWrite) return {};
      files[path].clear(); File result; result.data = &files[path]; result.maxWrite = maxWrite; return result;
    }
    if (!files.count(path)) return {};
    File result; result.data = &files[path]; return result;
  }
  bool exists(const char* path) { return files.count(path); }
  bool remove(const char* path) { return files.erase(path) != 0; }
  bool rename(const char* from, const char* to) {
    if (failInstall && std::string(from) == "/config.tmp") { failInstall = false; return false; }
    if (!files.count(from) || files.count(to)) return false;
    files[to] = files[from]; files.erase(from); return true;
  }
  bool format() { ++formats; files.clear(); return mounted = formatSucceeds; }
};
extern FSMock SPIFFS;
class PubSubClient {
 public:
  bool online = false, connectSucceeds = false;
  unsigned attempts = 0, loops = 0;
  std::vector<std::string> topics, publications;
  bool connected() { return online; }
  void loop() { ++loops; }
  bool connect(const char*) { ++attempts; return online = connectSucceeds; }
  bool connect(const char* id, const char*, const char*) { return connect(id); }
  void subscribe(const char* topic) { topics.push_back(topic); }
  void publish(const char* topic, const char* payload) { publications.push_back(std::string(topic) + ":" + payload); }
};
class WiFiManager { public: void resetSettings() {} };
