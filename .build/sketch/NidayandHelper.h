#line 1 "G:\\Meine Ablage\\Sicherungen\\Raspi-Sicherungen\\Node MCU ESP8266\\motor-on-roller-blind-ws-master\\v2\\motor_on_a_roller_blind-ws\\NidayandHelper.h"
#ifndef NidayandHelper_h
#define NidayandHelper_h

#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <WiFiManager.h>
#include <list>

class NidayandHelper {
 public:
  NidayandHelper();
  boolean beginStorage(uint32_t first, uint32_t last);
  boolean loadconfig();
  JsonVariantConst getconfig() const;
  boolean saveconfig(JsonVariantConst json);
  String mqtt_gettopic(String type);
  void mqtt_reconnect(PubSubClient& client);
  void mqtt_reconnect(PubSubClient& client, std::list<const char*> topics);
  void mqtt_reconnect(PubSubClient& client, String uid, String pwd);
  void mqtt_reconnect(PubSubClient& client, String uid, String pwd, std::list<const char*> topics);
  void mqtt_publish(PubSubClient& client, String topic, String payload);
  void resetsettings(WiFiManager& manager);
 private:
  // ArduinoJson 7 owns the parsed object AND its copied strings.
  JsonDocument _config;
  unsigned long _lastMqttAttempt;
};
#endif
