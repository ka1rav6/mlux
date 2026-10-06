/*
 *  This file contains all the types related to the geometry of panes/objects
 *  that will be drawn.
 * */

#ifndef MLUX_GEOMETRY_H
#define MLUX_GEOMETRY_H

#include "definitions.hpp"

namespace mlux {
// Basic struct to represent a point
struct Point {
    int x;
    int y;
    Point(int x, int y) : x(x), y(y) {}
    ~Point() = default;
};

// basic struct to represent the size of an object
struct Size {
    Height height;
    Width width;
    Size(Height h, Width w) : height(h), width(w) {}
    ~Size() = default;
};

// basic rectangle struct. A pane will be defined through a rectangle
// `contains` function used to check if the `Point` lies within the space
// where the rectangle is.
struct Rectangle {
    int x;
    int y;
    Height height;
    Width width;
    [[nodiscard]] bool contains(Point p) const;
    Rectangle(int x, int y, Height h, Width w)
        : x(x), y(y), height(h), width(w) {}
    ~Rectangle() = default;
};

// The whole pane geomentry is defined through one main rectangle (for now)
class PaneGeometry {
  public:
    /*
     * outer.x = column where the pane starts
     * outer.y = row where the pane starts
     * outer.width = number of columns occupied
     * outer.height = number of rows occupied
     *
     */
    Rectangle outer;
    PaneGeometry(Rectangle outer) : outer(outer) {}
    void resize();
};

} // namespace mlux

#endif
