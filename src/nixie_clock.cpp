#include "nixie_clock.h"

NixieClock::Clock::Clock() : display() {}

bool NixieClock::Clock::begin() {
    rtc_init();

    // Troubleshoot if mux wasn't found!
    // display.verbose(); 
    // display.scan();
    display.begin();
    return setTimeDate(NixieClock::DEFAULT_TIME); // Resets time to default value "2026-01-01 00:00:00"
}

bool NixieClock::Clock::setTimeDate(datetime_t new_time) {
    if (rtc_set_datetime(&new_time)) {
        show(NixieClock::TimeOrDate::TIME);
        return true;
    }
    return false;
}

void NixieClock::Clock::readBuffer(NixieClock::TimeOrDate requestType, NixieClock::digit_buffer& buffer) {
    datetime_t now;

    // Check if RTC is running; if not, return 000000
    if (rtc_get_datetime(&now)) {
        if (requestType == TimeOrDate::TIME) {
            // Time: HHMMSS
            buffer[0] = (now.hour / 10) + '0'; buffer[1] = (now.hour % 10) + '0';
            buffer[2] = (now.min / 10)  + '0'; buffer[3] = (now.min % 10)  + '0';
            buffer[4] = (now.sec / 10)  + '0'; buffer[5] = (now.sec % 10)  + '0';
        } else if (requestType == TimeOrDate::DATE) {
            // Date: YYMMDD
            buffer[0] = (now.year % 100 / 10) + '0'; buffer[1] = (now.year % 100 % 10) + '0';
            buffer[2] = (now.month / 10) + '0'; buffer[3] = (now.month % 10) + '0';
            buffer[4] = (now.day / 10)   + '0'; buffer[5] = (now.day % 10)   + '0';
        }
    }
}

void NixieClock::Clock::show(NixieClock::TimeOrDate timeOrDate) {
    readBuffer(timeOrDate, digitBuffer);
    display.printBuffer(digitBuffer);
}

NixieClock::MuxDisplay::MuxDisplay() : Adafruit_SSD1306(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire, -1) {
        mux_addr =      MUX_ADDR;
        display_addr =  DISPLAY_ADDR;
        size =          NIXIE_CLUSTER_SIZE;
        speed =         I2C_SPEED;
        _verbose =      false;
    }

bool NixieClock::MuxDisplay::begin() { 
    _verbose = false;

    // Unpack digit points to draw lines
    NixieDigit::unpackLines(linesUnpacked, DISPLAY_WIDTH);

    // Prepare I2C bus
    Wire.begin();
    Wire.setClock(speed);
    Wire.beginTransmission(mux_addr);

    // Initialize SSD1306 OLEDs
    for (uint8_t chan = 0; chan < size; chan++) {
        if (selectChannel(chan) && Adafruit_SSD1306::begin(SSD1306_SWITCHCAPVCC, display_addr)) {
            continue;
        } else {
            return false;
        }
    }
    return true;
}

bool NixieClock::MuxDisplay::selectChannel(uint8_t channel) {
    if (channel > size - 1) return false;
    Wire.beginTransmission(mux_addr);
    Wire.write(1 << channel); // Send bitmask to the control register
    // If transmission sucessful, record current channel
    if (Wire.endTransmission() == 0) {
        currentChannel = channel;
        if (_verbose) { 
            Serial.printf("Mux >> %02d.\n", currentChannel);
        };
        return true;
    } else return false;
}

bool NixieClock::MuxDisplay::scan() {
    // In verbose mode, adds a delay 5 sec. to give time for cable re-connection, running terminal etc.
    if (_verbose) { delay(5000); }
    for (uint8_t chan = 0; chan < size; chan++) {
        if (!selectChannel(chan)) {
            Serial.println("[ERROR] Could not connect to Multiplexer!");
            return false;
        }
        uint8_t connectedDevices = 0;
        for (uint8_t addr = 0x08; addr <= 0x77; addr++) {
        // Ignore the Mux's address
            if (addr == mux_addr) continue;
            Wire.beginTransmission(addr);
            if (Wire.endTransmission() == 0) {
                connectedDevices++;
                if (_verbose) { Serial.printf("Found I2C device (%#x) at channel %02d\n", addr, chan); }
            }
        }
        if (_verbose && connectedDevices == 0) { Serial.printf("No I2C devices found at channel %02d\n", chan); }
    }
    selectChannel(0);
    return true;
}

void NixieClock::MuxDisplay::verboseMode(bool flag) { _verbose = flag; }

void NixieClock::MuxDisplay::drawNumber(uint8_t position, uint8_t value) {
    // Implementation to update display with digit
    const auto& lines = linesUnpacked[value];
    selectChannel(position);
    clearDisplay();
    if (_verbose) { Serial.println(value); }
    for (auto& line : lines) {
        drawLine(line.x0, line.y0, line.x1, line.y1, SSD1306_WHITE);
        if (_verbose) {
            Serial.printf("x0 = %d, y0 = %d, x1 = %d, y1 = %d", line.x0, line.y0, line.x1, line.y1);
        }
    }
    display();
}

void NixieClock::MuxDisplay::printBuffer(const NixieClock::digit_buffer& buf) {
  for (uint8_t i = 0; i < size; i++) {
    uint8_t value = buf[i] - '0';
    drawNumber(i, value);
  }
}