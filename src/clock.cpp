#include "clock.h"

NixieClock::NixieClock() {
    memset(timeBuf, '0', 6);
    memset(dateBuf, '0', 6);
}

bool NixieClock::begin() {
    rtc_init();
    return setTimeDate(); // Resets time to default value "2026-01-01 00:00:00"
}

bool NixieClock::setTimeDate(datetime_t new_time) {
    return (rtc_set_datetime(&new_time));
}

bool NixieClock::update() {
    datetime_t now;
    // Check if RTC is running; if not, return false
    if (!rtc_get_datetime(&now)) {
        return false;
    }

    // Time: HHMMSS
    timeBuf[0] = (now.hour / 10) + '0'; timeBuf[1] = (now.hour % 10) + '0';
    timeBuf[2] = (now.min / 10)  + '0'; timeBuf[3] = (now.min % 10)  + '0';
    timeBuf[4] = (now.sec / 10)  + '0'; timeBuf[5] = (now.sec % 10)  + '0';

    // Date: YYMMDD
    int shortYear = now.year % 100;
    dateBuf[0] = (shortYear / 10) + '0'; dateBuf[1] = (shortYear % 10) + '0';
    dateBuf[2] = (now.month / 10) + '0'; dateBuf[3] = (now.month % 10) + '0';
    dateBuf[4] = (now.day / 10)   + '0'; dateBuf[5] = (now.day % 10)   + '0';

    return true;
}

