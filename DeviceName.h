#ifndef DEVICE_NAME_H
#define DEVICE_NAME_H
#include <stddef.h>

// A single DNS label usable by ESP8266 DHCP, mDNS and ArduinoOTA.
class DeviceName {
 public:
  static bool valid(const char* name) {
    if (!name || !*name) return false;
    size_t size = 0;
    for (; name[size]; ++size) {
      char c = name[size];
      if (size >= 31 || !((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                          (c >= '0' && c <= '9') || c == '-')) return false;
    }
    return name[0] != '-' && name[size-1] != '-';
  }
};
#endif
