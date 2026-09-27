/* lexer for a byte stream.
 * creates a csi node if it is called.
 * This lexer is only to the called ONCE the actual lexer realises the byte
 * stream has a csi escape sequence within it. That is when the lexer hands it
 * over to this lexer.
 * This lexer returns once the state of the machine is back to `ground` and
 * gives the CSI node of the escape sequence
 */
#ifndef MLUX_ESC_SEQ_PARSER
#define MLUX_ESC_SEQ_PARSER

#include "csi.hpp"

namespace mlux {
class esc_sequence_parser {
  public:
    esc_sequence_parser(const char *src, csi_parser_state *st);
    ~esc_sequence_parser() = default;

  private:
    const char *src;
    size_t current = 0;
    csi_parser_state *state;
    csi_node *output;
    [[nodiscard]] std::vector<int> split_params();
    [[nodiscard]] char peek() const;
    char advance();
    bool is_alpha(char c) const;
    bool is_int(char c) const;
    bool is_esc(char c) const;
    void parse();
};
} // namespace mlux

#endif
