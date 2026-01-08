#include "mux_display.h"

NixieMuxDisplay::NixieMuxDisplay(
        uint8_t mux_address, 
        uint8_t display_address, 
        uint8_t buffer_size, 
        uint32_t bus_speed
    )
    : SSD1306(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire, -1) {
        // I2C address of the Mux
        mux_addr = mux_address;
        // I2C address of the SSD1306   
        display_addr = display_address;
        // Amount of active channels
        size = buffer_size;
        // Setting I2C frequency
        speed = bus_speed;
        _verbose = false;
    }

bool NixieMuxDisplay::begin() { 
    _verbose = false;
    linesUnpacked = unpackNixieDigitLines();
    Wire.begin();
    Wire.setClock(speed);
    Wire.beginTransmission(mux_addr);
    for (uint8_t chan = 0; chan < size; chan++) {
        if (select(chan) && SSD1306.begin(SSD1306_SWITCHCAPVCC, display_addr)) {
            continue;
        } else {
            return false;
        }
    }
    return true;
}

bool NixieMuxDisplay::select(uint8_t channel) {
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

bool NixieMuxDisplay::scan() {
    // In verbose mode, adds a delay 5 sec. to give time for cable re-connection, running terminal etc.
    if (_verbose) { delay(5000); }
    for (uint8_t chan = 0; chan < size; chan++) {
        if (!select(chan)) {
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
    select(0);
    return true;
}

void NixieMuxDisplay::verbose(bool flag) { _verbose = flag; }

void NixieMuxDisplay::set(uint8_t position, uint8_t value) {
    // Implementation to update display with digit
    const auto& lines = linesUnpacked[value];
    select(position);
    SSD1306.clearDisplay();
    if (_verbose) { Serial.println(value); }
    for (auto& line : lines) {
        SSD1306.drawLine(line.x0, line.y0, line.x1, line.y1, SSD1306_WHITE);
        if (_verbose) {
            Serial.printf("x0 = %d, y0 = %d, x1 = %d, y1 = %d", line.x0, line.y0, line.x1, line.y1);
        }
    }
    SSD1306.display();
}

void NixieMuxDisplay::setBuf(char* buf) {
  for (uint8_t i = 0; i < size; i++) {
    uint8_t value = buf[i] - '0';
    set(i, value);
  }
}

nixieDigitSet unpackNixieDigitLines() {
    nixieDigitSet digits;
    for (auto& digit_points : NIXIE_DIGITS_POINTS) {
        nixieDigit lines = {};
        for (std::size_t i = 0; i + 1 < digit_points.size(); i++) {
            const uint16_t x0 = (digit_points[i] % GRID_WIDTH) * GRID_GAP;
            const uint16_t y0 = (digit_points[i] / GRID_WIDTH) * GRID_GAP;
            const uint16_t x1 = (digit_points[i + 1] % GRID_WIDTH) * GRID_GAP;
            const uint16_t y1 = (digit_points[i + 1] / GRID_WIDTH) * GRID_GAP;
            lines.push_back({x0, y0, x1, y1});            
        }
        digits.push_back(lines);
    }
    return digits;
};