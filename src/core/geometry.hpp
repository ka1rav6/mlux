#ifndef MLUX_GEOMETRY_H
#define MLUX_GEOMETRY_H

#include "definitions.hpp"

namespace mlux {
struct Point {
    int x;
    int y;
    Point() = default;
    ~Point() = default;
};

struct Size {
    Height height;
    Width width;
    Size() = default;
    ~Size() = default;
};

struct Rectangle {
    int x;
    int y;
    Width width;
    Height height;
    [[nodiscard]] bool contains(Point p) const;
};

struct PaneGeometry {
    Rectangle outer;
    Rectangle content;
    Rectangle border;
};
} // namespace mlux

#endif
