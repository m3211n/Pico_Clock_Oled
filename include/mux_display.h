#include <vector>
#include <Adafruit_SSD1306.h> // Includes Wire

#ifndef MUX_DISPLAY_H
#define MUX_DISPLAY_H


#define DISPLAY_WIDTH   128
#define DISPLAY_HEIGHT  32
#define GRID_WIDTH      9
#define GRID_GAP        15

#define DISPLAY_ADDR  0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define MUX_ADDR 0x70
#define NIXIE_CLUSTER_SIZE 6
#define I2C_SPEED 100000 // Hz

const std::vector<std::vector<uint8_t>> NIXIE_DIGITS_POINTS = {{
    {0, 8, 26, 18, 0},
    {7, 17, 9},
    {18, 0, 1, 23, 26, 8, 7},
    {0, 9, 20, 23, 14, 25, 26, 8, 7},
    {18, 26, 4, 3, 21},
    {0, 9, 20, 23, 5, 8, 26},
    {5, 23, 18, 0, 6, 17, 26},
    {0, 26, 8, 7},
    {0, 2, 24, 26, 8, 6, 20, 18, 0},
    {0, 9, 20, 26, 8, 3, 21}
}};

struct Line { uint16_t x0, y0, x1, y1; };

using nixieDigit = std::vector<Line>;
using nixieDigitSet = std::vector<nixieDigit>;

nixieDigitSet unpackNixieDigitLines();

class NixieMuxDisplay {

public:
    NixieMuxDisplay(
        uint8_t mux_address=MUX_ADDR, 
        uint8_t display_address=DISPLAY_ADDR, 
        uint8_t size=NIXIE_CLUSTER_SIZE, 
        uint32_t speed=I2C_SPEED
    );
    bool begin();
    // Select a channel with connected display
    // Operate display at selected channel
    void set(uint8_t position, uint8_t value);
    void setBuf(char* buf);
    // Utility
    bool scan();
    void verbose(bool flag=true);

private:
    Adafruit_SSD1306 SSD1306;
    uint8_t size;
    uint8_t mux_addr;
    uint8_t display_addr;
    uint32_t speed;
    uint8_t currentChannel;
    bool _verbose;
    bool select(uint8_t channel);
    nixieDigitSet linesUnpacked;
};


#endif