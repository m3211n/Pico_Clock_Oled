#include "nixie_clock.h"

volatile bool updateClock = false;
volatile bool showTime = true;

NixieClock::Clock nixieClock;
struct repeating_timer timer;

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
    nixieClock.show(showTime);
  }
}

