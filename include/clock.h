#include <Arduino.h>
#include "hardware/rtc.h"
#include "pico/util/datetime.h"

#ifndef CLOCK_H
#define CLOCK_H

inline const datetime_t defaultTime = {
    .year = 2026,
    .month = 1,
    .day = 1,
    .dotw = 0,
    .hour = 0,
    .min = 0,
    .sec = 0
};

class NixieClock {
public:
    NixieClock();
    bool begin();
    bool update();
    char timeBuf[6];
    char dateBuf[6];
    bool setTimeDate(datetime_t t=defaultTime);
};

#endif