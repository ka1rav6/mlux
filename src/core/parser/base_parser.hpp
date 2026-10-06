#ifndef MLUX_BASE_PARSER_H
#define MLUX_BASE_PARSER_H

#include "../terminal.hpp"
#include <span>

namespace mlux {

enum class ParserState {
    GROUND,
    CSI_ENTRY,
    CSI_MARKER,
    CSI_PARAM,
    CSI_INTERMEDIATE,
    CSI_FINAL,
    OSC_ENTRY, // TODO: write other states.
};

class BaseParser {
  public:
    BaseParser(Terminal &, ParserState *state);
    virtual void feed(std::span<const std::byte>);
    void processByte(uint8_t);

  private:
    ParserState *main_state;
    [[nodiscard]] bool is_alpha(char c);
    [[nodiscard]] bool is_int(char c);
    [[nodiscard]] bool is_esc(char c);
    char advance();
    char peek();

};

} // namespace mlux

#endif
