// Compile the ACTUAL persistence/MQTT helper against in-memory host peripherals.
#include "../NidayandHelper.h"
#include <assert.h>
#include <stdio.h>
ESPMock ESP;
WiFiMock WiFi;
FSMock SPIFFS;
uint32_t fakeMillis = 0;

int main() {
  NidayandHelper mountHelper;
  assert(mountHelper.beginStorage(0, 4096));
  assert(!SPIFFS.autoFormat && SPIFFS.formats == 0);
  SPIFFS.mounted = false;
  // A selected region containing ANY old data must never be reformatted.
  ESP.flash[4095] = 0;
  assert(!mountHelper.beginStorage(0, 4096) && SPIFFS.formats == 0);
  ESP.flash[4095] = 255; ESP.readFails = true;
  assert(!mountHelper.beginStorage(0, 4096) && SPIFFS.formats == 0);
  ESP.readFails = false;
  assert(!mountHelper.beginStorage(0, 4095) && SPIFFS.formats == 0);
  assert(mountHelper.beginStorage(0, 4096) && SPIFFS.formats == 1);
  puts("PASS: mount existing storage, refuse nonblank/failed/unaligned flash, initialize only erased storage");

  SPIFFS.files["/config.json"] = "{\"currentPosition\":123,\"maxPosition\":900,\"config_name\":\"test-blind\",\"mqtt_pwd\":\"test-only\"}";
  NidayandHelper helper;
  assert(helper.loadconfig());
  // ASan exercises the formerly dangling JSON pool/input after load returns.
  std::vector<std::string> churn(200, std::string(200, 'x'));
  JsonVariantConst loaded = helper.getconfig();
  assert(loaded["currentPosition"].as<long>() == 123);
  assert(std::string(loaded["config_name"].as<const char*>()) == "test-blind");
  assert(std::string(loaded["mqtt_pwd"].as<const char*>()) == "test-only");
  const std::string oldConfig = SPIFFS.files["/config.json"];

  JsonDocument candidate;
  candidate["currentPosition"] = 456;
  candidate["startPosition"] = -100;
  candidate["maxPosition"] = 1900;
  candidate["mqtt_pwd"] = "quotes-\"-and-backslash-\\";
  assert(helper.saveconfig(candidate.as<JsonVariantConst>()));
  assert(SPIFFS.files["/config.bak"] == oldConfig);
  const std::string goodConfig = SPIFFS.files["/config.json"];
  {
    NidayandHelper afterRestart;
    assert(afterRestart.loadconfig());
    JsonVariantConst restored = afterRestart.getconfig();
    assert(restored["startPosition"].is<long>() && restored["maxPosition"].is<long>());
    assert(restored["startPosition"].as<long>() == -100);
    assert(restored["maxPosition"].as<long>() == 1900);
  }
  puts("PASS: both saved endpoints reload unchanged in a fresh helper instance");
  assert(helper.loadconfig());
  assert(helper.getconfig()["currentPosition"].as<long>() == 456);
  assert(std::string(helper.getconfig()["mqtt_pwd"].as<const char*>()) == "quotes-\"-and-backslash-\\");

  SPIFFS.failOpenWrite = true;
  assert(!helper.saveconfig(candidate.as<JsonVariantConst>())); assert(SPIFFS.files["/config.json"] == goodConfig);
  SPIFFS.failOpenWrite = false; SPIFFS.maxWrite = 10;
  assert(!helper.saveconfig(candidate.as<JsonVariantConst>())); assert(SPIFFS.files["/config.json"] == goodConfig);
  assert(!SPIFFS.exists("/config.tmp")); SPIFFS.maxWrite = SIZE_MAX;
  SPIFFS.failInstall = true;
  assert(!helper.saveconfig(candidate.as<JsonVariantConst>())); assert(SPIFFS.files["/config.json"] == goodConfig);

  SPIFFS.files["/config.bak"] = goodConfig;
  SPIFFS.files["/config.json"] = "{broken";
  assert(helper.loadconfig()); assert(helper.getconfig()["currentPosition"].as<long>() == 456);
  for (const char* nonObject : {"null", "[]", "42"}) {
    SPIFFS.files["/config.json"] = nonObject;
    assert(helper.loadconfig());
    assert(helper.getconfig()["currentPosition"].as<long>() == 456);
  }
  SPIFFS.files.erase("/config.json");
  assert(helper.loadconfig()); // Interrupted rotation, only backup remains.
  SPIFFS.files["/config.json"] = std::string(3000, 'x');
  assert(helper.loadconfig()); // Oversize primary also falls back safely.
  SPIFFS.files.clear(); assert(!helper.loadconfig());
  puts("PASS: real helper JSON lifetime, roundtrip, escaping, write failures and backup recovery");

  PubSubClient client;
  fakeMillis = 4999; helper.mqtt_reconnect(client, "user", "test-only", {"test/in"});
  assert(client.attempts == 0);
  fakeMillis = 5000; helper.mqtt_reconnect(client, "user", "test-only", {"test/in"});
  assert(client.attempts == 1 && !client.online);
  fakeMillis = 5001; helper.mqtt_reconnect(client); assert(client.attempts == 1);
  fakeMillis = 10000; client.connectSucceeds = true;
  helper.mqtt_reconnect(client, "user", "test-only", {"test/in"});
  assert(client.attempts == 2 && client.online);
  assert(client.topics.size() == 1 && client.topics[0] == "test/in");
  assert(client.publications.size() == 1 && client.publications[0].find("/raw/esp8266/register:") == 0);
  helper.mqtt_reconnect(client); assert(client.loops == 1);
  client.online = false; WiFi.connection = 0; fakeMillis = 15000;
  helper.mqtt_reconnect(client); assert(client.attempts == 2);
  puts("PASS: real helper MQTT reconnect throttling, offline behavior, subscription and registration");
}
