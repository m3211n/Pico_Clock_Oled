#define VERBOSE_MODE 1

#include "nixie_clock.h"

volatile bool updateClock = false;
volatile bool showTime = true;

NixieClock::Clock nixieClock;
struct repeating_timer nixieTimer0, nixieTimer1;

bool clockUpdateTimerCallback(struct repeating_timer *t) {
  updateClock = true;
  return true;
}

bool clockModeTimerCallback(struct repeating_timer *t) {
  showTime = !showTime;
  showTime ? nixieClock.focusAt(0) : nixieClock.focusAt(1);
  return true;
}

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 1000);
  nixieClock.begin();
  add_repeating_timer_ms(-1000, clockUpdateTimerCallback, NULL, &nixieTimer0);
  add_repeating_timer_ms(-5000, clockModeTimerCallback, NULL, &nixieTimer1);
}

void loop() {
  if (updateClock) {
    updateClock = false;
    nixieClock.switchMode(showTime);
    nixieClock.refresh();
  }
}

