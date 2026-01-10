#include <array>
#include <vector>

#include <Arduino.h>

#include "Adafruit_SSD1306.h"
#include "hardware/rtc.h"
#include "pico/util/datetime.h"

#include "nixie_digit.h"

#ifndef NIXIE_CLOCK_H
#define NIXIE_CLOCK_H

// I2C hardware parameters for the I2C multiplexor (TCA9548 / PCA9548)
constexpr uint8_t MUX_ADDR = 0x70;
constexpr uint32_t I2C_SPEED = 100000; // Hz

// This is how many displays are connected to the mux
constexpr uint8_t NIXIE_CLUSTER_SIZE = 6;

// Display (SSD1306 OLED) parameters
constexpr uint8_t DISPLAY_ADDR = 0x3C; // < See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
constexpr int16_t DISPLAY_WIDTH = 128;
constexpr int16_t DISPLAY_HEIGHT = 32;


namespace NixieClock {
    using digit_buffer = std::array<char, 6>;

    const datetime_t DEFAULT_TIME = {
        .year = 2026,
        .month = 1,
        .day = 1,
        .dotw = 0,
        .hour = 0,
        .min = 0,
        .sec = 0
    };

    enum class TimeOrDate {
        TIME,
        DATE
    };
    // Manages multiplexor with displays connected to its channels
    class MuxDisplay : public Adafruit_SSD1306 {
    public:
        MuxDisplay();
        // Initializes multiplexor and displays
        bool begin();
        // Selects a channel with connected display and draws a digit in it
        void drawNumber(uint8_t position, uint8_t number);
        // Draws the entire buffer to all displays consequentially
        void printBuffer(const NixieClock::digit_buffer& buf);
        // Utility function to scan the I2C bus through each of the channels
        bool scan();
        // Enables verbose mode for multiplexor
        void verboseMode(bool flag);

    private:
        uint8_t size;
        uint8_t mux_addr;
        uint8_t display_addr;
        uint32_t speed;
        uint8_t currentChannel;
        bool _verbose;
        NixieDigit::DigitSet linesUnpacked;
        bool selectChannel(uint8_t channel);
    };

    // Clock manager
    class Clock {
    public:  
        Clock();

        bool begin();
        bool setTimeDate(datetime_t t);
        void show(NixieClock::TimeOrDate timeOrDate);

    private:
        NixieClock::MuxDisplay display;
        NixieClock::digit_buffer digitBuffer;

        void readBuffer(NixieClock::TimeOrDate dataType, NixieClock::digit_buffer& digitBuffer);
    };
}

#endif