#include "esc_sequence_parser.hpp"

namespace mlux {

char esc_sequence_parser::advance() {
    if (idx == bytes.size() - 1) {
        parsed = true;
        return '\0';
    }
    return bytes[idx++];
}
} // namespace mlux
