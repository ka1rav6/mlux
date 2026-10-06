#ifndef MLUX_TERMINAL_H
#define MLUX_TERMINAL_H

#include "definitions.hpp"
#include "geometry.hpp"
#include "screen.hpp"

#include <span>

namespace mlux {

class Terminal {
  public:
    Terminal(Size size);
    void feed(std::span<const std::byte> data);
    void resize(Size);
    Screen &screen();
    const Screen &screen() const;
    Cursor &cursor();

  private:
    TerminalParser _parser;
    Screen _screen;
    Cursor _cursor;
    TerminalModes _modes;
};

} // namespace mlux
