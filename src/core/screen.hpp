#ifndef MLUX_SCREEN_H
#define MLUX_SCREEN_H

#include "definitions.hpp"
#include "geometry.hpp"
#include <vector>

using TextAttributes = uint64_t; // going to bitmask different attributes

namespace mlux {
struct Cell {
    char32_t character;
    uint32_t foreground;
    uint32_t background;
    TextAttributes attributes;
};

class Screen {
  public:
    Screen(Size);
    Cell &at(uint16_t x, uint16_t y);
    void clear();
    void scrollUp();
    void scrollDown();
    void resize(Size);

    [[nodiscard]] Size size() const;

    // change this into a scrollback buffer later on
  private:
    std::vector<Cell> _cells;
    Size _size;
    std::vector<Cell> _scrollback;
};

} // namespace mlux

#endif
