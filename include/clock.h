#include <Arduino.h>
#include "hardware/rtc.h"
#include "pico/util/datetime.h"

#ifndef CLOCK_H
#define CLOCK_H

class NixieClock {
public:
    NixieClock();
    bool begin();
    bool update();
    char timeBuf[6];
    char dateBuf[6];
};

#endif