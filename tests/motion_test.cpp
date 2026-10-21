#include "../BlindMotion.h"
#include "../StatusSchedule.h"
#include <assert.h>
#include <stdio.h>
#include <initializer_list>

int main() {
  long value = 0;
  assert(BlindMotion::parseNumber("0", 0, 100, value) && value == 0);
  assert(BlindMotion::parseNumber("100", 0, 100, value) && value == 100);
  for (const char* bad : {"", "-1", "101", "abc", "50abc", "50.5", " 50", "+50", "999999999999999999999999"})
    assert(!BlindMotion::parseNumber(bad, 0, 100, value));
  assert(BlindMotion::parseNumber("-1000000000", -1000000000L, 1000000000L, value));
  assert(!BlindMotion::validLimits(0, 0));
  assert(!BlindMotion::validLimits(10, 0));
  assert(!BlindMotion::validLimits(-1000000001L, 50));

  BlindMotion m;
  m.start = -1000; m.end = 3000; m.current = -1000;
  assert(m.percent(-1000) == 0 && m.percent(1000) == 50 && m.percent(3000) == 100);
  assert(m.percent(-2000) == 0 && m.percent(4000) == 100);
  assert(m.goTo(75) && m.target == 2000 && m.direction() == 1);
  while (m.moving()) m.current += m.direction();
  assert(m.current == 2000 && !m.moving());
  m.stop();
  assert(m.goTo(0) && m.target == -1000 && m.direction() == -1);
  m.stop();
  assert(!m.moving() && m.target == m.current);
  assert(!m.goTo(-1) && !m.goTo(101));

  // Large nonzero offsets used to overflow the old 32-bit percent calculation.
  m.start = -1000000000L; m.end = 1000000000L;
  assert(m.goTo(100) && m.target == 1000000000L);
  assert(m.goTo(50) && m.target == 0);
  assert(m.percent(500000000L) == 75);
  m.current = 999999999L; m.manual(1);
  assert(m.moving()); m.current += m.direction();
  assert(!m.moving());
  m.current = -999999999L; m.manual(-1);
  m.current += m.direction(); assert(!m.moving());
  m.manual(1); assert(m.moving()); m.stop();
  assert(m.manualDirection == 0 && !m.automatic);

  // Editing endpoints keeps the physical step coordinate; no unexpected move.
  m.current = 500; m.start = 100; m.end = 900; m.stop();
  assert(m.percent(m.current) == 50);
  m.start = 200; m.end = 800;
  assert(m.current == 500 && !m.moving());
  assert(m.goTo(100) && m.target == 800);
  StatusSchedule schedule;
  schedule.sent(100, true); // Immediate start status.
  assert(!schedule.mqttDue(5099, true));
  assert(schedule.mqttDue(5100, true));
  assert(!schedule.mqttDue(5100, false));
  assert(schedule.webDue(1100, true));
  schedule.sent(1100, false); // Web status must not postpone the MQTT interval.
  assert(schedule.mqttDue(5100, true));
  schedule.sent(5100, true);
  assert(!schedule.mqttDue(5101, true));
  schedule.sent(UINT32_MAX - 2000U, true);
  assert(!schedule.mqttDue(2998, true));
  assert(schedule.mqttDue(2999, true)); // millis() rollover after ~49.7 days.
  puts("Motion/timer tests passed: strict commands, offsets, limits, targets, stop, overflow, 5-second MQTT schedule and millis rollover.");
}
