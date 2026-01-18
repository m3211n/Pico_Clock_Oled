#define VERBOSE_MODE 1

#include "nixie_clock.h"
#include "WiFi.h"
#include <pico/util/datetime.h>
#include "time.h"

#define MY_TIMEZONE "CET-1CEST,M3.5.0/2,M10.5.0/3"

volatile bool updateClock = false;
volatile bool showTime = true;
volatile uint8_t focus = 0;

constexpr datetime_t DEFAULT_TIME = {
    .year = 2026,
    .month = 1,
    .day = 17,
    .dotw = 5,
    .hour = 18,
    .min = 13,
    .sec = 0
};

const char* SSID = "Home Sweet Home 6G";
const char* PASS = "82840813.home";

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

  Serial.println("[INFO] ---- NIXIE CLOCK INITIALIZATION SEQUENCE START ----");

  Serial.printf("Connecting to %s ", SSID);
  // Turn LED on to indicate sync process start
  digitalWrite(LED_BUILTIN, HIGH);
  WiFi.begin(SSID, PASS);

  // --- STEP 1: Start NTP Sync ---
  // The NTP object is built into the WiFi library for Pico W
  Serial.println("Starting NTP sync...");
  NTP.begin("pool.ntp.org", "time.nist.gov");

  // --- STEP 2: Wait for Time ---
  // Earle Philhower core has a helper function to block until time is set
  Serial.print("Waiting for NTP response");
  NTP.waitSet(); 
  Serial.println(" -> Time Received!");

  // --- STEP 3: Apply Timezone Rule ---
  // We use standard C environment variables for this
  setenv("TZ", MY_TIMEZONE, 1);
  tzset(); // Apply the rule

  // Turn off LED to indicate sync process end
  digitalWrite(LED_BUILTIN, LOW);

  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) { // Wait until we have a realistic time
      delay(500);
      Serial.print(".");
      now = time(nullptr);
  }
  Serial.println("\nTime Synced!");

  struct tm timeinfo;
  localtime_r(&now, &timeinfo);

  datetime_t dt;
  dt.year  = timeinfo.tm_year + 1900; // tm_year is years since 1900
  dt.month = timeinfo.tm_mon + 1;     // tm_mon is 0-11
  dt.day   = timeinfo.tm_mday;        // tm_mday is 1-31
  dt.dotw  = timeinfo.tm_wday;        // tm_wday is 0-6 (0=Sunday), same as datetime_t
  dt.hour  = timeinfo.tm_hour;
  dt.min   = timeinfo.tm_min;
  dt.sec   = timeinfo.tm_sec;

  nixieClock.begin(dt);
  Serial.print("[INFO] Starting update timer...");
  add_repeating_timer_ms(-1000, clockUpdateTimerCallback, NULL, &nixieTimer0);
  Serial.println("DONE!");

  Serial.println("[INFO] ---- NIXIE CLOCK INITIALIZATION SEQUENCE DONE! ----");


  // add_repeating_timer_ms(-5000, clockModeTimerCallback, NULL, &nixieTimer1);
}

void loop() {
  if (updateClock) {
    updateClock = false;
    // nixieClock.switchMode(showTime);
    // nixieClock.focusAt(focus);
    Serial.println("[INFO] Main loop update triggered!");
    nixieClock.refresh();
  }
}

