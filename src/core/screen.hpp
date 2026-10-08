#ifndef MLUX_SCREEN_H
#define MLUX_SCREEN_H

#include "definitions.hpp"
#include "geometry.hpp"

#include <cstdint>
#include <vector>

namespace mlux
{

using TextAttributes = uint64_t; // going to bitmask different attributes

struct Cell
{
    char32_t character = U' ';
    uint32_t foreground = 0;
    uint32_t background = 0;
    TextAttributes attributes = 0;
};

// The screen that is going to be visible to the user.
class Screen
{
public:
    explicit Screen(Size size);

    [[nodiscard]] Cell& at(uint16_t x, uint16_t y);
    [[nodiscard]] const Cell& at(uint16_t x, uint16_t y) const;
    void clear();
    void scrollUp();
    void scrollDown();
    void resize(Size size);

    [[nodiscard]] Size size() const;
    // change this into a scrollback buffer later on
private:
    std::vector<Cell> _cells;
    Size _size;
    std::vector<Cell> _scrollback;
};

} // namespace mlux

#endif
