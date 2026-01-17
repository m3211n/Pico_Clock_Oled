#include "nixie_clock.h"

NixieClock::Clock::Clock() {
    mode_ = true;
    updateClock_ = false;
}

bool NixieClock::Clock::begin(datetime_t initTime) {
    multiDisplay_.begin();
    if (!rtc_running()) {
        rtc_init();
        return setDateTime(initTime);
    } 
    return true;
}

bool NixieClock::Clock::setDateTime(datetime_t new_time) {
    if (rtc_set_datetime(&new_time)) {
        refresh();
        return true;
    }
    return false;
}

void NixieClock::Clock::switchMode(bool mode) {
    #if VERBOSE_MODE > 0
    Serial.printf("Switching mode to %s \n", mode_ ? "time" : "date");
    #endif
    mode_ = mode;
}

void NixieClock::Clock::focusAt(uint8_t index) {
    unFocus();
    multiDisplay_.focus(index * 2, true);
    multiDisplay_.refresh(index * 2);
    multiDisplay_.focus(index * 2 + 1, true);
    multiDisplay_.refresh(index * 2 + 1);
}

void NixieClock::Clock::unFocus() {
    for (uint8_t i = 0; i < CLUSTER_SIZE; i++) {
        if (multiDisplay_.isFocused(i)) {
            multiDisplay_.focus(i, false);
            multiDisplay_.refresh(i);
        }
    }
}

void NixieClock::Clock::updateDisplayPair_(uint8_t index, uint8_t value) {
    multiDisplay_.setDigit(index * 2, value / 10);
    multiDisplay_.setDigit(index * 2 + 1, value % 10);
};

void NixieClock::Clock::updateDisplaysRegister_() {
    updateDisplayPair_(0, mode_ ? now_.hour : now_.year % 100);
    updateDisplayPair_(1, mode_ ? now_.min : now_.month);
    updateDisplayPair_(2, mode_ ? now_.sec : now_.day);
}

void NixieClock::Clock::refresh() {
    if (rtc_get_datetime(&now_)) {
        #if VERBOSE_MODE > 0
        Serial.printf("Current mode is %s \n", mode_ ? "time" : "date");
        #endif
        updateDisplaysRegister_();
    }
}


NixieClock::MultiDisplay::MultiDisplay() : Adafruit_SSD1306(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire, -1) {
    mux_addr_ =     MUX_ADDR;
    display_addr_ = DISPLAY_ADDR;
    size_ =         CLUSTER_SIZE;
    bus_speed_ =    I2C_SPEED;
    currentChannel_ = 8; // Initialize with value bigger than 7 (maximum channel address)
}

bool NixieClock::MultiDisplay::begin() {
    // Unpack digit points to draw lines
    NixieDigit::unpackLines(linesUnpacked_, DISPLAY_WIDTH);

    // Prepare I2C bus
    Wire.begin();
    Wire.setClock(bus_speed_);
    Wire.beginTransmission(mux_addr_);

    // Initialize SSD1306 OLEDs
    for (uint8_t chan = 0; chan < size_; chan++) {
        if (selectChannel_(chan) && Adafruit_SSD1306::begin(i2caddr=DISPLAY_ADDR)) {
            displayRegister_[chan].digit = 0;
            refresh(chan);
            dim(true);
            focus(chan, false);
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

        #if VERBOSE_MODE > 0
            Serial.printf("Mux >> %02d.\n", currentChannel_);
        #endif

        return true;
    } else return false;
}

bool NixieClock::MultiDisplay::setDigit(uint8_t position, uint8_t value) {
    if (displayRegister_[position].digit != value) {

        #if VERBOSE_MODE > 0
            Serial.printf("Assigning %d to position %02d (current value: %d) \n", value, position, displayRegister_[position].digit);
        #endif

        displayRegister_[position].digit = value;
        refresh(position);
        return true;
    } else return false;
}

void NixieClock::MultiDisplay::refresh(uint8_t position) {
    // Implementation to update display with digit
    const auto& lines = linesUnpacked_[displayRegister_[position].digit];
    selectChannel_(position);
    clearDisplay();

    #if VERBOSE_MODE > 0
        Serial.printf("Printing %d at position %02d \n", displayRegister_[position].digit, position);
    #endif
    
    for (auto& line : lines) { 
        drawLine(line.x0 + 6, line.y0, line.x1 + 6, line.y1, SSD1306_WHITE);
    }
    
    // invertDisplay(displayRegister_[position].focused);
    fillRect(0, 0, 3, 31, displayRegister_[position].focused ? SSD1306_WHITE : SSD1306_BLACK);

    dim(!displayRegister_[position].focused);
    // setRotation(1);
    // drawChar(0, 0, displayRegister_[position].value + '0', 1, 0, 6);
    display();
}

void NixieClock::MultiDisplay::focus(uint8_t position, bool focused) {
    displayRegister_[position].focused = focused;
    // paint horizontal lines with black to dim even more;
}

bool NixieClock::MultiDisplay::isFocused(uint8_t position) {
    return displayRegister_[position].focused;
}

bool NixieClock::MultiDisplay::scan() {
    // In verbose mode, adds a delay 5 sec. to give time for cable re-connection, running terminal etc.
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

                #if VERBOSE_MODE > 0
                Serial.printf("Found I2C device (%#x) at channel %02d\n", addr, chan);
                #endif

            }
        }

        #if VERBOSE_MODE > 0
        if (connectedDevices == 0) { Serial.printf("No I2C devices found at channel %02d\n", chan); }
        #endif

    }
    selectChannel_(0);
    return true;
}