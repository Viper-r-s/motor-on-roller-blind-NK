#ifndef STATUS_SCHEDULE_H
#define STATUS_SCHEDULE_H
#include <stdint.h>

class StatusSchedule {
 public:
  uint32_t lastMqtt = 0;
  uint32_t lastWeb = 0;
  bool mqttDue(uint32_t now, bool moving) const {
    return moving && uint32_t(now - lastMqtt) >= 5000U;
  }
  bool webDue(uint32_t now, bool moving) const {
    return moving && uint32_t(now - lastWeb) >= 1000U;
  }
  void sent(uint32_t now, bool mqtt) {
    lastWeb = now;
    if (mqtt) lastMqtt = now;
  }
};
#endif
