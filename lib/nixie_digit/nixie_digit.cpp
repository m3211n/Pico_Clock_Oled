#include "nixie_digit.h"

void NixieDigit::unpackLines(NixieDigit::DigitSet& digits, uint16_t canvasWidth) {
    uint16_t gridGap = canvasWidth / (GRID_WIDTH-1) - 1;
    uint8_t index = 0;
    for (auto& digit_points : NixieDigit::POINTS) {
        NixieDigit::Digit lines = {};
        for (std::size_t i = 0; i + 1 < digit_points.size(); i++) {
            const uint16_t x0 = (digit_points[i] % GRID_WIDTH) * gridGap;
            const uint16_t y0 = (digit_points[i] / GRID_WIDTH) * gridGap;
            const uint16_t x1 = (digit_points[i + 1] % GRID_WIDTH) * gridGap;
            const uint16_t y1 = (digit_points[i + 1] / GRID_WIDTH) * gridGap;
            lines.push_back({x0, y0, x1, y1});            
        }
        digits[index] = lines; 
        index++;
    }
};