#pragma once

#include "definitions.hpp"
#include "geometry.hpp"
#include "screen.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>

namespace mlux {

class Parser;

struct Cursor {
    uint16_t row = 0;
    uint16_t column = 0;
    bool visible = true;
};

struct TerminalModes {
    bool applicationCursorKeys = false;
    bool autoWrap = true;
    bool originMode = false;
    bool insertMode = false;
    bool bracketedPaste = false;
};

class Terminal {
  public:
    explicit Terminal(Size size);
    ~Terminal();

    Terminal(const Terminal &) = delete;
    Terminal &operator=(const Terminal &) = delete;

    void feed(std::span<const std::byte> data);
    void resize(Size size);

    [[nodiscard]] Screen &screen();
    [[nodiscard]] const Screen &screen() const;
    [[nodiscard]] Cursor &cursor();
    [[nodiscard]] const Cursor &cursor() const;
    [[nodiscard]] TerminalModes &modes();
    [[nodiscard]] const TerminalModes &modes() const;

  private:
    std::unique_ptr<Parser> _parser;
    Screen _screen;
    Cursor _cursor;
    TerminalModes _modes;
};

} // namespace mlux
