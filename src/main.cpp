#include "nixie_clock.h"

volatile bool updateClock = false;
volatile bool showDate = false;

struct repeating_timer timer;

NixieClock::Clock        nixieClock;

bool timer_callback(struct repeating_timer *t) {
  updateClock = true;
  return true;
}

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 1000);
  nixieClock.begin();
  add_repeating_timer_ms(-1000, timer_callback, NULL, &timer);
}

void loop() {
  if (updateClock) {
    updateClock = false;
    nixieClock.show(showDate ? NixieClock::TimeOrDate::DATE : NixieClock::TimeOrDate::TIME);
  }
}

