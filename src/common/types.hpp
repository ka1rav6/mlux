#ifndef MLUX_COMMON_TYPES_HPP
#define MLUX_COMMON_TYPES_HPP

#include <cstddef>
#include <cstdint>
#include <iostream>

namespace mlux {

struct Position {
    int x = 0;
    int y = 0;

    Position() = default;
    Position(int x, int y) : x(x), y(y) {}

    bool operator==(const Position &other) const {
        return x == other.x && y == other.y;
    }

    friend std::ostream &operator<<(std::ostream &os, const Position &pos) {
        return os << "Position(x=" << pos.x << ", y=" << pos.y << ")";
    }
};

struct Size {
    std::size_t width = 0;
    std::size_t height = 0;

    Size() = default;
    Size(std::size_t width, std::size_t height)
        : width(width), height(height) {}

    bool operator==(const Size &other) const {
        return width == other.width && height == other.height;
    }

    friend std::ostream &operator<<(std::ostream &os, const Size &size) {
        return os << "Size(width=" << size.width << ", height=" << size.height
                  << ")";
    }
};

} // namespace mlux

#endif // MLUX_COMMON_TYPES_HPP
