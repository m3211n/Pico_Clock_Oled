#define VERBOSE_MODE 1

#include "nixie_clock.h"

volatile bool updateClock = false;
volatile bool showTime = true;
volatile uint8_t focus = 0;

constexpr datetime_t DEFAULT_TIME = {
    .year = 2026,
    .month = 1,
    .day = 17,
    .dotw = 5,
    .hour = 15,
    .min = 38,
    .sec = 0
};

NixieClock::Clock nixieClock;
struct repeating_timer nixieTimer0, nixieTimer1;

bool clockUpdateTimerCallback(struct repeating_timer *t) {
  updateClock = true;
  return true;
  // toggleFocus();
}

void toggleFocus() {
  if (focus == 2) { focus = 0; } 
  else if (focus == 0) { focus = 1; } 
  else if (focus == 1) { focus = 2; }
}

bool clockModeTimerCallback(struct repeating_timer *t) {
  showTime = !showTime;
  return true;
}

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 1000);
  nixieClock.begin(DEFAULT_TIME);
  add_repeating_timer_ms(-1000, clockUpdateTimerCallback, NULL, &nixieTimer0);
  add_repeating_timer_ms(-5000, clockModeTimerCallback, NULL, &nixieTimer1);
}

void loop() {
  if (updateClock) {
    updateClock = false;
    // nixieClock.switchMode(showTime);
    // nixieClock.focusAt(focus);
    nixieClock.refresh();
  }
}

