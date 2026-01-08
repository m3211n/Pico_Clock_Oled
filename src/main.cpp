#include <Arduino.h>

#include "mux_display.h"
#include "clock.h"

volatile bool updateDisplay = false;
volatile bool showDate = false;

struct repeating_timer timer;

bool timer_callback(struct repeating_timer *t) {
  updateDisplay = true;
  return true;
}

NixieMuxDisplay muxDisplay;
NixieClock nixieClock;

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 1000);
  muxDisplay.begin();
  nixieClock.begin();

  //////// Troubleshoot if mux wasn't found! ////////

  // muxDisplay.verbose(); 
  // muxDisplay.scan();

  muxDisplay.setBuf(nixieClock.timeBuf); // Shows initial buffer (000000)
  add_repeating_timer_ms(-1000, timer_callback, NULL, &timer);
}

void loop() {
  // Serial.print("updateDisplay == "); Serial.println(updateDisplay);
  if (updateDisplay) {
    updateDisplay = false;
    nixieClock.update();
    if (showDate) {
      muxDisplay.setBuf(nixieClock.dateBuf);
    } else {
      muxDisplay.setBuf(nixieClock.timeBuf);
    }
  }
}

