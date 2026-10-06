/* lexer for a byte stream.
 * creates a csi node if it is called.
 * This lexer is only to the called ONCE the actual lexer realises the byte
 * stream has a csi escape sequence within it. That is when the lexer hands it
 * over to this lexer.
 * This lexer returns once the state of the machine is back to `ground` and
 * gives the CSI node of the escape sequence
 * It is a continuous finite state machine
 */
#pragma once
#include "csi.hpp"
#include <ostream>
#include <string>
#include <vector>

namespace mlux {
class esc_sequence_parser {
  public:
    esc_sequence_parser() = default;
    ~esc_sequence_parser() = default;

  private:
    std::string bytes;
    size_t idx;
    csi_parser_state current_state;
    csi_node *current_node;
    bool parsed = false;
    [[nodiscard]] std::vector<int> split_params();
    [[nodiscard]] char peek() const;
    char advance();
    [[nodiscard]] static bool is_alpha(char c);
    [[nodiscard]] static bool is_int(char c);
    [[nodiscard]] static bool is_esc(char c);
    void parse();
    esc_sequence_parser &operator<<(const std::string &more_bytes) {
        if (parsed)
            bytes = more_bytes;
        else
            bytes += more_bytes;
        return *this;
    }
    esc_sequence_parser &operator<<(std::string &&more_bytes) {
        if (parsed)
            bytes = std::move(more_bytes);
        else
            bytes += more_bytes;
        return *this;
    }
};
} // namespace mlux
