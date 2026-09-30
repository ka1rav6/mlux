/*
 * contains some basic utilities required within the code.
 * this includes Position struct and Size struct
 * */

#pragma once

#include <cstddef>
#include <cstdint>
#include <iostream>
// TODO: create copy and move constructors for these structs
namespace mlux {

using PaneId = uint32_t;
using Coord = int32_t;

struct Position {
    Coord x = 0;
    Coord y = 0;
    Position() = default;
    Position(Coord x, Coord y) : x(x), y(y) {}
    bool operator==(const Position &other) const {
        return x == other.x && y == other.y;
    }
    // for debugging and logging purposes
    friend std::ostream &operator<<(std::ostream &os, const Position &pos) {
        return os << "Position(x=" << pos.x << ", y=" << pos.y << ")";
    }
};

struct PixelSize {
    Coord width = 0;
    Coord height = 0;

    PixelSize() = default;
    PixelSize(Coord w, Coord h) : width(w), height(h) {}

    bool operator==(const PixelSize &other) const {
        return width == other.width && height == other.height;
    }

    friend std::ostream &operator<<(std::ostream &os, const PixelSize &size) {
        return os << "PixelSize(width=" << size.width
                  << ", height=" << size.height << ")";
    }
};

struct CellSize {
    Coord rows = 0;
    Coord columns = 0;

    CellSize() = default;
    CellSize(Coord r, Coord c) : rows(r), columns(c) {}

    bool operator==(const CellSize &other) const {
        return rows == other.rows && columns == other.columns;
    }

    friend std::ostream &operator<<(std::ostream &os, const CellSize &size) {
        return os << "CellSize(rows=" << size.rows
                  << ", columns=" << size.columns << ")";
    }
};

} // namespace mlux
