#pragma once

#include "base_parser.hpp"
#include "csi_parser.hpp"

#include <cstddef>
#include <cstdint>
#include <span>

namespace mlux {

class Terminal;

class Parser {
  public:
    explicit Parser(Terminal &terminal);

    Parser(const Parser &) = delete;
    Parser &operator=(const Parser &) = delete;

    void feed(std::span<const std::byte> data);

  private:
    void processByte(uint8_t byte);

    Terminal &_terminal;
    ParserState _state = ParserState::Ground;

    CSIParser _csiParser;
};

} // namespace mlux
