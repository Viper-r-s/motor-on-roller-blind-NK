#include "../DeviceName.h"
#include <assert.h>
#include <string>
#include <stdio.h>
int main() {
  for (const char* name : {"a", "0", "bedroom-blind", "Nico-Office-1", "room--2", "123"}) assert(DeviceName::valid(name));
  for (const char* name : {"", "-room", "room-", "room.local", "two rooms", "roller_blind", "röllchen", "<script>", "a\nb", " room", "room "}) assert(!DeviceName::valid(name));
  assert(!DeviceName::valid(nullptr));
  assert(DeviceName::valid(std::string(31,'a').c_str()));
  assert(!DeviceName::valid(std::string(32,'a').c_str()));
  puts("PASS: valid DNS names, boundary lengths and invalid/unsafe device names");
}
