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
    size_t width = 0;
    size_t height = 0;
    size_t rows = 0;
    size_t columns = 0;

    enum SizeType {
        SIZE_WH,
        SIZE_RC,
    };

    Size() = default;
    Size(size_t w_or_r, size_t h_or_c, SizeType type) {
        if (type == SIZE_RC) {
            this->rows = w_or_r;
            this->columns = h_or_c;
        }
        if (type == SIZE_WH) {
            this->width = w_or_r;
            this->height = h_or_c;
        }
    }

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
