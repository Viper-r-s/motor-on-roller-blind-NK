#ifndef BLIND_MOTION_H
#define BLIND_MOTION_H

#include <stdint.h>
#include <stdlib.h>
#include <errno.h>

// Software coordinates only; this project has no encoder or limit switches.
class BlindMotion {
 public:
  static const long coordinateLimit = 1000000000L;
  long current = 0;
  long start = 0;
  long end = 2000000;
  long target = 0;
  int manualDirection = 0;
  bool automatic = false;

  static bool validLimits(long first, long last) {
    return first >= -coordinateLimit && last <= coordinateLimit && first < last;
  }
  static bool parseNumber(const char* text, long minimum, long maximum, long& result) {
    if (!text || !*text) return false;
    const char* p = text;
    if (*p == '-') ++p;
    if (!*p) return false;
    for (; *p; ++p) if (*p < '0' || *p > '9') return false;
    errno = 0;
    char* tail;
    long value = strtol(text, &tail, 10);
    if (errno == ERANGE || *tail || value < minimum || value > maximum) return false;
    result = value;
    return true;
  }
  int percent(long value) const {
    if (!validLimits(start, end)) return 0;
    int64_t result = (int64_t(value) - start) * 100 / (int64_t(end) - start);
    return result < 0 ? 0 : result > 100 ? 100 : int(result);
  }
  bool goTo(int percentage) {
    if (percentage < 0 || percentage > 100 || !validLimits(start, end)) return false;
    target = start + (int64_t(end) - start) * percentage / 100;
    automatic = true;
    manualDirection = 0;
    return true;
  }
  void manual(int direction) {
    automatic = false;
    manualDirection = direction;
    target = current;
  }
  void stop() {
    automatic = false;
    manualDirection = 0;
    target = current;
  }
  int direction() const {
    int sign = automatic ? (target > current ? 1 : target < current ? -1 : 0) : manualDirection;
    if ((sign > 0 && current >= coordinateLimit) || (sign < 0 && current <= -coordinateLimit)) return 0;
    return sign;
  }
  bool moving() const { return direction() != 0; }
};
#endif
