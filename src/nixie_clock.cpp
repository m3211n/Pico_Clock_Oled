#include "nixie_clock.h"

NixieClock::Clock::Clock() : multiDisplay_() {}

bool NixieClock::Clock::begin() {
    rtc_init();
    multiDisplay_.begin();
    return setTimeDate(NixieClock::DEFAULT_TIME);
}

bool NixieClock::Clock::setTimeDate(datetime_t new_time) {
    if (rtc_set_datetime(&new_time)) {
        show(NixieClock::TimeOrDate::TIME);
        return true;
    }
    return false;
}

void NixieClock::Clock::readBuffer_(NixieClock::TimeOrDate requestType) {
    // Check if RTC is running; if not, return 000000
    if (rtc_get_datetime(&now_)) {
        if (requestType == TimeOrDate::TIME) {
            // Time: HHMMSS
            digitBuffer_[0] = (now_.hour / 10) + '0';
            digitBuffer_[1] = (now_.hour % 10) + '0';
            digitBuffer_[2] = (now_.min / 10) + '0';
            digitBuffer_[3] = (now_.min % 10) + '0';
            digitBuffer_[4] = (now_.sec / 10) + '0';
            digitBuffer_[5] = (now_.sec % 10) + '0';
        } else if (requestType == TimeOrDate::DATE) {
            // Date: YYMMDD
            digitBuffer_[0] = (now_.year % 100 / 10) + '0';
            digitBuffer_[1] = (now_.year % 100 % 10) + '0';
            digitBuffer_[2] = (now_.month / 10) + '0';
            digitBuffer_[3] = (now_.month % 10) + '0';
            digitBuffer_[4] = (now_.day / 10) + '0';
            digitBuffer_[5] = (now_.day % 10) + '0';
        }
    }
}

void NixieClock::Clock::show(NixieClock::TimeOrDate timeOrDate) {
    readBuffer_(timeOrDate);
    multiDisplay_.printBuffer(digitBuffer_);
}

NixieClock::MultiDisplay::MultiDisplay() : Adafruit_SSD1306(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire, -1) {
    mux_addr_ =     MUX_ADDR;
    display_addr_ = DISPLAY_ADDR;
    size_ =         CLUSTER_SIZE;
    bus_speed_ =    I2C_SPEED;
    verbose_ =      false;
}

bool NixieClock::MultiDisplay::begin() { 
    verbose_ = false;

    // Unpack digit points to draw lines
    NixieDigit::unpackLines(linesUnpacked_, DISPLAY_WIDTH);

    // Prepare I2C bus
    Wire.begin();
    Wire.setClock(bus_speed_);
    Wire.beginTransmission(mux_addr_);

    // Initialize SSD1306 OLEDs
    for (uint8_t chan = 0; chan < size_; chan++) {
        if (selectChannel_(chan) && Adafruit_SSD1306::begin(SSD1306_SWITCHCAPVCC, display_addr_)) {
            continue;
        } else {
            return false;
        }
    }
    return true;
}

bool NixieClock::MultiDisplay::selectChannel_(uint8_t channel) {
    if (channel > size_ - 1) return false;
    Wire.beginTransmission(mux_addr_);
    Wire.write(1 << channel); // Send bitmask to the control register
    // If transmission sucessful, record current channel
    if (Wire.endTransmission() == 0) {
        currentChannel_ = channel;
        if (verbose_) { 
            Serial.printf("Mux >> %02d.\n", currentChannel_);
        };
        return true;
    } else return false;
}

void NixieClock::MultiDisplay::drawNumber(uint8_t position, uint8_t value) {
    // Implementation to update display with digit
    const auto& lines = linesUnpacked_[value];
    selectChannel_(position);
    clearDisplay();
    if (verbose_) { Serial.println(value); }
    for (auto& line : lines) {
        drawLine(line.x0, line.y0, line.x1, line.y1, SSD1306_WHITE);
        if (verbose_) {
            Serial.printf("x0 = %d, y0 = %d, x1 = %d, y1 = %d", line.x0, line.y0, line.x1, line.y1);
        }
    }
    display();
}

void NixieClock::MultiDisplay::printBuffer(const NixieClock::digit_buffer& buf) {
  for (uint8_t i = 0; i < size_; i++) {
    uint8_t value = buf[i] - '0';
    drawNumber(i, value);
  }
}

bool NixieClock::MultiDisplay::scan() {
    // In verbose mode, adds a delay 5 sec. to give time for cable re-connection, running terminal etc.
    if (verbose_) { delay(5000); }
    for (uint8_t chan = 0; chan < size_; chan++) {
        if (!selectChannel_(chan)) {
            Serial.println("[ERROR] Could not connect to Multiplexer!");
            return false;
        }
        uint8_t connectedDevices = 0;
        for (uint8_t addr = 0x08; addr <= 0x77; addr++) {
        // Ignore the Mux's address
            if (addr == mux_addr_) continue;
            Wire.beginTransmission(addr);
            if (Wire.endTransmission() == 0) {
                connectedDevices++;
                if (verbose_) { Serial.printf("Found I2C device (%#x) at channel %02d\n", addr, chan); }
            }
        }
        if (verbose_ && connectedDevices == 0) { Serial.printf("No I2C devices found at channel %02d\n", chan); }
    }
    selectChannel_(0);
    return true;
}

void NixieClock::MultiDisplay::verboseMode(bool flag) { verbose_ = flag; }