#include "NidayandHelper.h"
#include <memory>
#include <new>

NidayandHelper::NidayandHelper() : _lastMqttAttempt(0) {}

boolean NidayandHelper::beginStorage(uint32_t first, uint32_t last) {
  // Never auto-format an existing volume when an IDE flash-layout selection is wrong.
  SPIFFSConfig options;
  options.setAutoFormat(false);
  SPIFFS.setConfig(options);
  if (SPIFFS.begin()) return true;
  if (last <= first || (first & 3U) || (last & 3U)) return false;
  // Fresh chips may initialize storage, but only if the ENTIRE selected area is erased.
  uint32_t words[64];
  for (uint32_t address = first; address < last;) {
    size_t length = last - address;
    if (length > sizeof(words)) length = sizeof(words);
    if (!ESP.flashRead(address, words, length)) return false;
    for (size_t i = 0; i < length / sizeof(words[0]); ++i) {
      if (words[i] != UINT32_MAX) return false;
    }
    address += length;
    yield();
  }
  return SPIFFS.format() && SPIFFS.begin();
}

boolean NidayandHelper::loadconfig() {
  const char* paths[] = {"/config.json", "/config.bak"};
  for (const char* path : paths) {
    _config.clear();
    File file = SPIFFS.open(path, "r");
    if (!file || file.size() == 0 || file.size() > 2048) continue;
    // Keep the 2 KB read buffer off the tiny ESP8266 stack.
    size_t size = file.size();
    std::unique_ptr<char[]> data(new (std::nothrow) char[size + 1]);
    if (!data) { file.close(); return false; }
    size_t count = file.readBytes(data.get(), size);
    file.close();
    if (count != size) continue;
    data[size] = '\0';
    // Const input requests owned string copies; no reference to this temporary buffer escapes.
    DeserializationError error = deserializeJson(_config, static_cast<const char*>(data.get()), size);
    if (!error && _config.is<JsonObject>()) return true;
  }
  return false;
}

JsonVariantConst NidayandHelper::getconfig() const { return _config.as<JsonVariantConst>(); }

boolean NidayandHelper::saveconfig(JsonVariantConst json) {
  // Write and close a complete replacement before touching the previous file.
  File file = SPIFFS.open("/config.tmp", "w");
  if (!file) return false;
  size_t expected = measureJson(json);
  size_t written = serializeJson(json, file);
  file.flush();
  file.close();
  if (written != expected) {
    SPIFFS.remove("/config.tmp");
    return false;
  }
  if (SPIFFS.exists("/config.json")) {
    if (SPIFFS.exists("/config.bak") && !SPIFFS.remove("/config.bak")) return false;
    if (!SPIFFS.rename("/config.json", "/config.bak")) return false;
  }
  if (!SPIFFS.rename("/config.tmp", "/config.json")) {
    if (SPIFFS.exists("/config.bak")) SPIFFS.rename("/config.bak", "/config.json");
    return false;
  }
  return true;
}

String NidayandHelper::mqtt_gettopic(String type) {
  return "/raw/esp8266/" + String(ESP.getChipId()) + "/" + type;
}

void NidayandHelper::mqtt_reconnect(PubSubClient& client) {
  mqtt_reconnect(client, "", "");
}
void NidayandHelper::mqtt_reconnect(PubSubClient& client, std::list<const char*> topics) {
  mqtt_reconnect(client, "", "", topics);
}
void NidayandHelper::mqtt_reconnect(PubSubClient& client, String uid, String pwd) {
  mqtt_reconnect(client, uid, pwd, {});
}
void NidayandHelper::mqtt_reconnect(PubSubClient& client, String uid, String pwd, std::list<const char*> topics) {
  if (client.connected()) { client.loop(); return; }
  if (WiFi.status() != WL_CONNECTED) return;
  unsigned long now = millis();
  if (now - _lastMqttAttempt < 5000UL) return;
  _lastMqttAttempt = now;
  String id = "ESPClient-" + String(ESP.getChipId());
  bool connected = uid.length() ? client.connect(id.c_str(), uid.c_str(), pwd.c_str()) : client.connect(id.c_str());
  if (!connected) return;
  for (const char* topic : topics) client.subscribe(topic);
  String registration = "{\"id\":\"" + String(ESP.getChipId()) + "\",\"ip\":\"" + WiFi.localIP().toString() + "\"}";
  mqtt_publish(client, "/raw/esp8266/register", registration);
}

void NidayandHelper::mqtt_publish(PubSubClient& client, String topic, String payload) {
  if (client.connected()) client.publish(topic.c_str(), payload.c_str());
}
void NidayandHelper::resetsettings(WiFiManager& manager) {
  SPIFFS.format();
  manager.resetSettings();
}
