#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

namespace mlux {

class Terminal;

enum class ParserState : uint8_t {
    Ground,
    CSI,
    OSC,
    // TODO: Handle more
};

class BaseParser {
  public:
    BaseParser(Terminal &terminal, ParserState &state)
        : _terminal(terminal), _state(state) {}

    virtual ~BaseParser() = default;

    BaseParser(const BaseParser &) = delete;
    BaseParser &operator=(const BaseParser &) = delete;

    virtual void feed(std::span<const std::byte> data) = 0;

  protected:
    Terminal &_terminal;
    ParserState &_state;

    void changeState(ParserState state) { _state = state; }
};

} // namespace mlux
