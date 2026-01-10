#include <array>
#include <Arduino.h>

#include "Adafruit_SSD1306.h"
#include "hardware/rtc.h"
#include "nixie_digit.h"

#ifndef NIXIE_CLOCK_H
#define NIXIE_CLOCK_H

// I2C hardware parameters for the I2C multiplexor (TCA9548 / PCA9548)
constexpr uint8_t MUX_ADDR = 0x70;
constexpr uint32_t I2C_SPEED = 100000; // Hz

// This is how many displays are connected to the mux
constexpr uint8_t CLUSTER_SIZE = 6; // Max: 8

// Display (SSD1306 OLED) parameters
constexpr uint8_t DISPLAY_ADDR = 0x3C; // < See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
constexpr int16_t DISPLAY_WIDTH = 128;
constexpr int16_t DISPLAY_HEIGHT = 32;

namespace NixieClock {
    using DigitBuffer = std::array<char, 6>;

    constexpr datetime_t DEFAULT_TIME = {
        .year = 2026,
        .month = 1,
        .day = 1,
        .dotw = 0,
        .hour = 0,
        .min = 0,
        .sec = 0
    };

    // Multiplexer with displays connected to its channels
    class MultiDisplay : public Adafruit_SSD1306 {
    public:
        MultiDisplay();
        // Initializes multiplexor and displays
        bool begin();
        // Selects a channel with connected display and draws a digit in it
        void printNumber(uint8_t position, uint8_t number);
        // Draws the entire buffer to all displays consequentially
        void printBuffer();
        // Utility function to scan the I2C bus through each of the channels
        bool scan();
        // Enables verbose mode for multiplexor
        void verboseMode(bool flag);

        NixieClock::DigitBuffer digitBuffer;

    private:
        bool selectChannel_(uint8_t channel);

        uint8_t  size_;
        uint8_t  mux_addr_;
        uint8_t  display_addr_;
        uint32_t bus_speed_;
        uint8_t  currentChannel_;
        bool     verbose_;

        NixieDigit::DigitSet linesUnpacked_;
    };

    // Clock manager
    class Clock {
    public:  
        Clock();

        bool begin();
        bool setTimeDate(datetime_t t);
        // If flag set to 'true', then the time will be shown. 'false' - date.
        void show(bool showTime);

    private:
        datetime_t now_;

        NixieClock::MultiDisplay multiDisplay_;
        
        void updateBuffer_(
            NixieClock::DigitBuffer& buf,
            bool showTime
        );
        void updatePair_(
            NixieClock::DigitBuffer& buf,
            uint8_t index,
            uint8_t value
        );
    };
}

#endif